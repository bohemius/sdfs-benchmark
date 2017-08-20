#include "mbed.h"
#include "SDFileSystem.h"
#define DEBUG
#include "hl_debug.h"
#include <vector>
#include <string>


Timer timer;
DigitalIn button(p21, PullUp);
SDFileSystem sd(p11, p12, p13, p21, "sd", p15, SDFileSystem::SWITCH_NEG_NO, 80000000);

char buffer[4096];

void writeTest()
{
    //Test write performance by creating a 1MB file
    INFO("Testing %iB write performance...", sizeof(buffer));
    FileHandle* file = sd.open("Test File.bin", O_WRONLY | O_CREAT | O_TRUNC);
    if (file != NULL) {
        timer.start();
        for (int i = 0; i < (1048576 / sizeof(buffer)); i++) {
            if (file->write(buffer, sizeof(buffer)) != sizeof(buffer)) {
                timer.stop();
                ERR("write error!");
                timer.reset();
                return;
            }
        }
        timer.stop();
        if (file->close()) {
            ERR("failed to close file!");
        } else {
            INFO("done!\n\tResult: %.2f MB/s", 1 / (timer.read_us() / 1000000.0));
        }
        timer.reset();
    } else {
        ERR("failed to create file!");
    }
}

void readTest()
{
    //Test read performance by reading the 1MB file created by writeTest()
    INFO("Testing %iB read performance...", sizeof(buffer));
    FileHandle* file = sd.open("Test File.bin", O_RDONLY);
    if (file != NULL) {
        timer.start();
        int iterations = 0;
        while (file->read(buffer, sizeof(buffer)) == sizeof(buffer))
            iterations++;
        timer.stop();
        if (iterations != (1048576 / sizeof(buffer)))
            {ERR("read error!");}
        else if (file->close())
            {ERR("failed to close file!");}
        else if (sd.remove("Test File.bin"))
            {ERR("failed to delete file!");}
        else
            {INFO("done!\n\tResult: %.2f MB/s", 1 / (timer.read_us() / 1000000.0));}
        timer.reset();
    } else {
        ERR("failed to open file!");
    }
}

void print_contents() {
	//vector<string> contents;

	INFO("Reading webfs directory");
	DirHandle* dir = sd.opendir("webfs");
	struct dirent *dirEntry;
	if (dir != NULL) {
		//read all directory and file names in current directory into filename vector
		while ((dirEntry = dir->readdir()) != NULL) {
			//contents.push_back(string(dirEntry->d_name));
			INFO("%s ",dirEntry->d_name);
		}
	} else {
		ERR("Error opening directory %s", dir);
	}
	dir->closedir();
}

int main()
{
    //Configure CRC, large frames, and write validation
    INFO("Setting CRC");
    sd.crc(true);
    INFO("Setting large frames");
    sd.large_frames(true);
    INFO("Setting write validation");
    sd.write_validation(true);

    //Fill the buffer with random data for the write test
    srand(time(NULL));
    for (int i = 0; i < sizeof(buffer); i++)
        buffer[i] = rand();

    wait(1);
    
    //Make sure a card is present
    if (!sd.card_present()) {
        ERR("No card present!");
        exit(1);
    }

    //Try to mount the SD card
    INFO("Mounting SD card...");
    if (sd.mount() != 0) {
        ERR("failed!");
        exit(1);
    }
    INFO("success!");

    //Display the card type
    INFO("\tCard type: ");
    SDFileSystem::CardType cardType = sd.card_type();
    if (cardType == SDFileSystem::CARD_NONE) {
        INFO("None");
    } else if (cardType == SDFileSystem::CARD_MMC) {
        INFO("MMC");
    } else if (cardType == SDFileSystem::CARD_SD) {
        INFO("SD");
    } else if (cardType == SDFileSystem::CARD_SDHC) {
        INFO("SDHC");
    } else {
        INFO("Unknown");
    }

    //Display the card capacity
    INFO("\tSectors: %u", sd.disk_sectors());
    INFO("\tCapacity: %.1fMB", sd.disk_sectors() / 2048.0);

    /*//Format the card
    printf("Formatting SD card...");
    if (sd.format() != 0) {
        printf("failed!\n");
        continue;
    }
    printf("success!\n");*/

    print_contents();

    //Perform a read/write test
    writeTest();
    readTest();

    //INFO("try create file the old fashioned way");
     //   FILE *fp = fopen("/sd/sdtest.txt", "w");
     //   if(fp == NULL) {
     //   	ERR("Could not open file for write\n");
     //   } else {
     //       fprintf(fp, "Hello fun Xadow SD Card World!");
      //      fclose(fp);
      //  }

    //Unmount the SD card
    sd.unmount();
    INFO("Exiting");
    exit(0);
}

