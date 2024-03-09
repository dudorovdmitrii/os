#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdbool.h>
#include <regex.h>

// #pragma GCC diagnostic ignored "-Wformat-overflow="

#define FILE_REGEX "^/?/([a-zA-Z0-9_.-]+/)+[a-zA-Z0-9_-]+\\.[a-zA-Z0-9_-]+$"
#define FILE_PATH_SIZE 100
#define FILE_BUFFER_SIZE 1000
#define FILE_SIZE_SIZE 1000
#define LINE_SIZE 1000
// #define RESULT_PATH "/Users/dudorov-dmitr/Applications/osLab1/result.archive.txt"
// #define RESULT_PATH "/Users/dudorov-dmitr/Applications/osLab1/result.archive.txt"
#define ARCHIVE_EXTENSION "archive.txt"
#define ARCHIVE_EXTENSION_SIZE 11

void archivate(char *pathToInputDir, char* inputDir, FILE* ouputFile, bool *isFirst);
void dearchivate(char *archivePath, char *pathToOutputDir);

int main() {
    int key; 
    
    printf("1. Архивация\n2. Разархивация\nВыбор: ");
    scanf("%i", &key);

    switch (key) {
      case 1: {
        FILE *output;
        char pathToInputDir[FILE_PATH_SIZE];
        printf("Введите путь до директории для архивации: ");
        scanf("%s", pathToInputDir);

        char pathCopy[FILE_PATH_SIZE];
        strcpy(pathCopy, pathToInputDir);

        char inputDir[FILE_PATH_SIZE];
        char *nextDir = strtok(pathCopy, "/");
        while (nextDir != NULL) {
          sprintf(inputDir, "%s", nextDir);
          nextDir = strtok(NULL, "/");
        }

        output = fopen("result.archive.txt", "w");
        if (!output) {
          printf("Не удалось создать архив!\n");
          break;
        }

        bool isFirst = true;

        archivate(pathToInputDir, inputDir, output, &isFirst);
        break;
      }
      case 2: {
        char pathToArchive[FILE_PATH_SIZE];
        printf("Введите путь до архива: ");
        scanf("%s", pathToArchive);

        char pathToOutputDir[FILE_PATH_SIZE];
        printf("Введите путь до директории, куда разорхивировать файлы: ");
        scanf("%s", pathToOutputDir);

        dearchivate(pathToArchive, pathToOutputDir);
        break;
      }
      default: {
        printf("Такого варианта нет!");
      }
    }
    
    return 0;
}

void emptyString(char (*string)[], int size) {
  for (int j = 0; j < size; j++) {
    (*string)[j] = '\0';
  }
}

void dearchivate(char *archivePath, char *pathToOutputDir) {
  char archivePathCopy[FILE_PATH_SIZE];
  strcpy(archivePathCopy, pathToOutputDir);

  char *currentDir = strtok(archivePathCopy, "/");
  char currentDirFullPath[FILE_PATH_SIZE];
  sprintf(currentDirFullPath, "/%s", currentDir);

  while (currentDir != NULL) {
    DIR *dir = opendir(currentDirFullPath);

    if (!dir) {
      int dirCreationResult = mkdir(currentDirFullPath, 0777);

      if (dirCreationResult != 0) {
        printf("Не удалось создать директорию %s!\n", currentDirFullPath);
        return;
      }
    } 
    else {
      closedir(dir);
    }

    currentDir = strtok(NULL, "/");
    sprintf(currentDirFullPath, "%s/%s", currentDirFullPath, currentDir);
  }

  FILE *archiveFile = fopen(archivePath, "rb");

  if (!archiveFile) {
    printf("Архива %s не существует!\n", archivePath);
    return;
  }

  char *archiveBuffer = 0;
  long archiveBufferSize;

  int streamSetOffsetResult = fseek(archiveFile, 0, SEEK_END);
  archiveBufferSize = ftell(archiveFile);
  int streamResetOffsetResult = fseek(archiveFile, 0, SEEK_SET);

  if (streamSetOffsetResult == -1 || streamResetOffsetResult == -1) {
    printf("Не удалось определить размер файла %s!\n", archivePath);
    return;
  }

  archiveBuffer = malloc(archiveBufferSize);

  if (!archiveBuffer) {
    printf("Не удалось создать буфер файла %s!\n", archivePath);
    return;
  }

  fread(archiveBuffer, sizeof(char), archiveBufferSize, archiveFile);

  // Буфер текущего файла
  char fileBuffer[FILE_BUFFER_SIZE] = "";
  int fileBufferIndex = 0;

  // Путь до текущего файла
  char filePath[FILE_PATH_SIZE] = "";
  int filePathIndex = 0;
  bool isFilePathRetrieveOn = true;

  // Размер текущего файла
  char fileSize[FILE_SIZE_SIZE] = "";
  int fileSizeIndex = 0;
  bool isFileSizeRetrieveOn = false;

  // Текущая строка
  char line[LINE_SIZE];
  int lineIndex = 0;

  regex_t fileRegex;
  int i = 0;

  while(i < archiveBufferSize) {
    if (archiveBuffer[i] == '|') {
      if (i > 0) {
        // printf("filePath = %s\n", filePath);
        // printf("fileSize = %s\n", fileSize);
        // printf("line = %s\n", line);
        // printf("fileBuffer = %s", fileBuffer);

        // Нужно обрезать \n в конце буфера, так как при архивации добавляется дополнительный \n к контенту файла
        for (int j = FILE_BUFFER_SIZE; j >= 0; j--) {
          if (fileBuffer[j] == '\n') {
            fileBuffer[j] = '\0';
            break;
          }
        }

        char *currentPathSlice = strtok(filePath, "/");
        char currentPath[FILE_PATH_SIZE];
        sprintf(currentPath, "%s", pathToOutputDir);

        while (currentPathSlice != NULL) {
          sprintf(currentPath, "%s/%s", currentPath, currentPathSlice);

          int compiledRegex;
          compiledRegex = regcomp(&fileRegex, FILE_REGEX, REG_EXTENDED); 

          if(compiledRegex != 0) {
            printf("Регулярное выражение не валидно!\n");
          }
          else {
            compiledRegex = regexec(&fileRegex, currentPath, 0, NULL, 0); 

            if(!compiledRegex) {
              // currentPath содержит путь до файла
              FILE *file = fopen(currentPath, "w");
              fputs(fileBuffer, file);
              fclose(file);
            }
            else {
              // currentPath содержит путь до директории
              DIR *dir = opendir(currentPath);
              if (!dir) {
                mkdir(currentPath, 0777);
              }
              else {
                closedir(dir);
              }
            }
          }

          regfree(&fileRegex);
          currentPathSlice = strtok(NULL, "/");
        }
      }

      // Следующая строка - путь файла
      isFilePathRetrieveOn = true;

      // Обнуление буфера
      emptyString(&fileBuffer, FILE_BUFFER_SIZE);
      fileBufferIndex = 0;

      // Обнуление пути до файла
      emptyString(&filePath, FILE_PATH_SIZE);
      filePathIndex = 0;

      // Обнуление размера файла
      emptyString(&fileSize, FILE_SIZE_SIZE);
      fileSizeIndex = 0;

      i+=2;
      continue;
    }

    line[lineIndex++] = archiveBuffer[i];

    if (archiveBuffer[i] == '\n') {
      bool isContentLine = isFilePathRetrieveOn == false && isFileSizeRetrieveOn == false;

      if (isContentLine) {
        // Добавление строки в буфер файла
        int k = 0;
        for (int j = fileBufferIndex; j < lineIndex + fileBufferIndex; j++) {
          fileBuffer[j] = line[k++]; 
        }
        fileBufferIndex = fileBufferIndex + lineIndex;
      }

      // Обнуление строки
      emptyString(&line, LINE_SIZE);
      lineIndex = 0;

      if (isFilePathRetrieveOn) {
        // Следующая строка - размер файла
        isFilePathRetrieveOn = false;
        isFileSizeRetrieveOn = true;
      }
      else if (isFileSizeRetrieveOn) {
        // Следующая строка - контент файла
        isFileSizeRetrieveOn = false;
      }

      i++;
      continue;
    }

    if (isFilePathRetrieveOn) {
      // Заполнение пути до файла
      filePath[filePathIndex++] = archiveBuffer[i];
    }
    
    if (isFileSizeRetrieveOn) {
      // Заполнение размера файла
      fileSize[fileSizeIndex++] = archiveBuffer[i];
    }
    
    i++;
  }
}

void archivate(char *currentPath, char* inputDir, FILE* ouputFile, bool *isFirst) {
  DIR *dir = opendir(currentPath);

  if (!dir) {
    // Обработка файла

    char *fileBuffer = 0;
    long fileSize;
    FILE *file = fopen(currentPath, "rb");

    if (!file) {
      printf("Не удалось открыть %s", currentPath);
      return;
    }

    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    fileBuffer = malloc(fileSize);

    if (!fileBuffer) {
      printf("Не удалось создать буфер для %s", currentPath);
      return;
    } 

    fread(fileBuffer, sizeof(char), fileSize, file);
    fclose(file);

    // Заполнение пути до файла относительно inputDir 
    char relativePath[FILE_PATH_SIZE] = ""; 

    char currentPathCopy[FILE_PATH_SIZE] = "";
    strcpy(currentPathCopy, currentPath);

    char *currentPathSlice = strtok(currentPathCopy, "/");
    bool isRelativePathOn = false;

    while (currentPathSlice != NULL) {
       if (!strcmp(currentPathSlice, inputDir)) {
        isRelativePathOn = true;
      }

      if (isRelativePathOn) {
        sprintf(relativePath, "%s/%s", relativePath, currentPathSlice);
      }
      
      currentPathSlice = strtok(NULL, "/");
    }

    // Заполнение размера файла
    char size[FILE_SIZE_SIZE] = ""; 
    sprintf(size, "%ld\n", fileSize);

    // Заполнение контента файла
    char content[FILE_BUFFER_SIZE] = "";

    int lastRelativePathIndex = 0;
    char fileEnd[FILE_PATH_SIZE] = "";

    bool isArchive = true;

    while (lastRelativePathIndex < FILE_PATH_SIZE && relativePath[lastRelativePathIndex] != '\0') {
      lastRelativePathIndex++;
    }

    if (lastRelativePathIndex >= ARCHIVE_EXTENSION_SIZE) {
      int k = 0;
      for (int j = lastRelativePathIndex - ARCHIVE_EXTENSION_SIZE; j < lastRelativePathIndex; j++) {
        fileEnd[k++] = relativePath[j];
      }
    }

    for (int j = 0; j < ARCHIVE_EXTENSION_SIZE; j++) {
      if (fileEnd[j] != ARCHIVE_EXTENSION[j]) {
        isArchive = false;
        break;
      }
    }

    if (!*isFirst) {
      content[0] = '\n';
    }

    if (isArchive) {
        sprintf(content, "%s%s", content, fileBuffer);
    }
    else {
        sprintf(content, "%s%s\n%s%s\n|", content, relativePath, size, fileBuffer);
    }

    fputs(content, ouputFile);

    if (*isFirst) {
      *isFirst = false;
    }

    return;
  }

  // ОБработка директории
  char nextPath[FILE_PATH_SIZE];
  struct dirent *dp;

  while ((dp = readdir(dir)) != NULL) {
    // Проходимся по содержимому директрии dir
    char itemName[FILE_PATH_SIZE];
    sprintf(itemName, "%s", dp->d_name);

    if (strcmp(itemName, ".") != 0 && strcmp(itemName, "..") != 0) {
        // Формирование пути (nextPath) до файла/директории внутри dir
        strcpy(nextPath, currentPath);
        strcat(nextPath, "/");
        strcat(nextPath, itemName);

        archivate(nextPath, inputDir, ouputFile, isFirst);
    }
  }

  closedir(dir);
}
