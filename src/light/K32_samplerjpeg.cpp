/*
  K32_samplerjpeg.cpp
  Created by RIRI, april 2020.
  Released under GPL v3.0
*/

#include "Arduino.h"
#include "Wire.h"
#include "SD.h"
#include "K32_samplerjpeg.h"

K32_samplerjpeg::K32_samplerjpeg(const int SD_PIN[4])
{
    this->lock = xSemaphoreCreateMutex();

    // Start SD
    if (!SD.exists("/"))
    {
        SPI.begin(SD_PIN[2], SD_PIN[1], SD_PIN[0]);
        if (SD.begin(SD_PIN[3]))
            LOG("samplerjpeg SD card OK");
        else
            LOG("samplerjpeg SD card ERROR");
    }
    LOG("samplerjpeg SD scan");
    int countFile = 0;
    File root = SD.open("/");
    while (true)
    {
        File entry = root.openNextFile();
        if (!entry)
            break;
        LOG(entry.name());
        countFile += 1;
        entry.close();
    }
    root.close();
    LOGF("samplerjpeg SD %i files found.", countFile);
    LOG("");

    // Scan samples
    this->scanjpeg();
};

void K32_samplerjpeg::scanjpeg()
{
    // Checking task
    xTaskCreate(this->taskjpeg,
                "samplerjpeg_task",
                4096,
                (void *)this,
                1, // priority
                NULL);
}

String K32_samplerjpeg::path(int bank, int fich)
{
    xSemaphoreTake(this->lock, portMAX_DELAY);
    if (bank < 0)
    {
        bank = this->_bank;
    }

    String path = "/" + this->pad3(bank) + "/" + this->pad3(fich);

    if (this->samplesjpeg[bank][fich][0] > 1)
    {
        path += String(this->samplesjpeg[bank][fich]);
    }
    xSemaphoreGive(this->lock);

    if (SD.exists(path))
    {
        return path;
    }
    LOGF("SAMPLERjpeg: %s not found..\n", path.c_str());
    return "";
}

void K32_samplerjpeg::bank(int bank)
{
    xSemaphoreTake(this->lock, portMAX_DELAY);
    this->_bank = bank;
    xSemaphoreGive(this->lock);
}

int K32_samplerjpeg::bank()
{
    int b = 0;
    xSemaphoreTake(this->lock, portMAX_DELAY);
    b = this->_bank;
    xSemaphoreGive(this->lock);
    return b;
}

int K32_samplerjpeg::size(byte fich, byte bank)
{
    int sizeN = 0;
    String path = this->path(fich, bank);
    if (SD.exists(path))
        sizeN = SD.open(path).size();
    return sizeN;
}

void K32_samplerjpeg::remove(byte fich, byte bank)
{
    String path = this->path(fich, bank);
    if (SD.exists(path))
    {
        SD.remove(path);
        xSemaphoreTake(this->lock, portMAX_DELAY);
        this->samplesjpeg[bank][fich][0] = 0;
        xSemaphoreGive(this->lock);
        LOG("deleted " + path);
    }
}

String K32_samplerjpeg::pad3(int input)
{
    char bank[4];
    bank[3] = 0;
    bank[0] = '0' + input / 100;
    bank[1] = '0' + (input / 10) % 10;
    bank[2] = '0' + input % 10;
    return String(bank);
}

/*
 *   PRIVATE
 */

void K32_samplerjpeg::taskjpeg(void *parameter)
{
    LOG("samplerjpeg TASK ");

    K32_samplerjpeg *that = (K32_samplerjpeg *)parameter;
    TickType_t xFrequency = pdMS_TO_TICKS(5);

    xSemaphoreTake(that->lock, portMAX_DELAY);

    // Init Alias array
    for (byte bank = 0; bank < DMX_MAX_BANK; bank++)
        for (byte fich = 0; fich < DMX_MAX_FICH; fich++)
            for (byte k = 0; k < DMX_MAX_TITLE; k++)
                that->samplesjpeg[bank][fich][k] = 0;

    // // Check Bank dirs
    LOG("\nSAMPLERjpeg: Scanning...");
    for (byte i = 0; i < DMX_MAX_BANK; i++)
    {
        if (SD.exists("/" + that->pad3(i)))
        {
            LOG("  - scanning bank " + that->pad3(i));
            File dir = SD.open("/" + that->pad3(i));

            // Check fichs files
            while (true)
            {
                File entry = dir.openNextFile();
                if (!entry)
                {
                    break;
                }
                if (!entry.isDirectory())
                {
                    int fich = 0;
                    fich = (entry.name()[5] - '0') * 100 + (entry.name()[6] - '0') * 10 + (entry.name()[7] - '0');
                    if (fich < DMX_MAX_FICH)
                    {
                        that->samplesjpeg[i][fich][0] = 1; // put a stamp even if alias is empty
                        byte k = 0;
                        while ((k < DMX_MAX_TITLE) && (entry.name()[8 + k] != 0))
                        {
                            that->samplesjpeg[i][fich][k] = entry.name()[8 + k];
                            k++;
                        }
                    }
                    vTaskDelay(xFrequency);
                    LOG("File found: " + String(entry.name()) + " size=" + String(entry.size()));
                }
            }
            dir.close();
        }
    }
    xSemaphoreGive(that->lock);
    LOG("SAMPLERjpeg: ready.");

    vTaskDelete(NULL);
};
