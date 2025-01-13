#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <cstdlib>
#include <android/log.h>

bool searchForPatternInFile(const std::string& filename, const std::string& pattern) {
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        return false;
    }

    const size_t bufferSize = 4096;
    char buffer[bufferSize];
    ssize_t bytesRead;

    std::regex re(pattern, std::regex::icase);
    std::string fileContents;

    while ((bytesRead = read(fd, buffer, bufferSize)) > 0) {
        fileContents.append(buffer, bytesRead);
    }

    close(fd);

    return std::regex_search(fileContents, re);
}

bool copyFile(const std::string& source, const std::string& destination) {
    int srcFd = open(source.c_str(), O_RDONLY);
    int destFd = open(destination.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (srcFd == -1 || destFd == -1) {
        return false;
    }

    char buffer[4096];
    ssize_t bytesRead;
    while ((bytesRead = read(srcFd, buffer, sizeof(buffer))) > 0) {
        if (write(destFd, buffer, bytesRead) != bytesRead) {
            close(srcFd);
            close(destFd);
            return false;
        }
    }

    close(srcFd);
    close(destFd);

    return true;
}

void rebootSystem() {

    int result = system("reboot");
}

int main() {
    std::string bootFile = "/dev/block/by-name/boot";
    std::string pattern = "edgars@";
    std::string bootImg = "/data/local/tmp/boot.img";
    
    if (searchForPatternInFile(bootFile, pattern)) {
        if (copyFile(bootImg, bootFile)) {
            rebootSystem();
        }
    }

    return 0;
}
