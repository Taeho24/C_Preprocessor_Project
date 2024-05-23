#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

// 이미 include된 라이브러리를 저장하는 스택 생성
typedef struct IncludeStack {
    char library[MAX_LINE_LENGTH];
    struct IncludeStack* next;
} icStack;

icStack* includeTop = NULL;

void push(char* library) {
    icStack* node = (icStack*)malloc(sizeof(icStack));

    strcpy_s(node->library, MAX_LINE_LENGTH, library);

    node->next = includeTop;
    includeTop = node;
}


// icStack 메모리 해제
void clearStack() {
    while (includeTop != NULL) {
        icStack* node = (icStack*)malloc(sizeof(icStack));

        node = includeTop;
        includeTop = includeTop->next;

        free(node);
    }
}

// 이미 include된 라이브러리인지 확인
bool isIncluded(char* library) {
    icStack* tmp = includeTop;

    while (tmp != NULL) {
        // include가 확인 되면 true
        if (strcmp(tmp->library, library) == 0) {
            return true;
        }
        tmp = tmp->next;
    }

    return false;
}

// #include 처리
bool acceptInclude(char* line) {
    if (strstr(line, "#include") == NULL) return false;

    char macro[MAX_LINE_LENGTH];
    char postfix[3];

    strncpy_s(macro, line, 8);
    strncpy_s(postfix, line + strlen(line) - 3, 3);

    if (strcmp(macro, "#include") == 0 && strcmp(postfix, ".h\"") == 0) return true;
    else return false;
}

// #define 처리
bool acceptDefine(char* line) {
    if (strstr(line, "#define") == NULL) return false;

    char macro[MAX_LINE_LENGTH];

    strncpy_s(macro, line, 7);

    if (strcmp(macro, "#define") == 0) return true;
    else return false;
}

// 한 줄에서 주석을 처리하는 함수
bool remove_comments(char* line) {
    if (strstr(line, "//") == NULL) return false;

    char macro[MAX_LINE_LENGTH];

    strncpy_s(macro, line, 2);

    if (strcmp(macro, "//") == 0) return true;
    else return false;
}

// 전처리 전 파일 경로 추출
// 파일 경로 + 파일 이름 -> 파일 경로 추출
char* extractFilePath(char* fileName) {
    int copySize = 0;
    char filePath[MAX_LINE_LENGTH] = "";

    for (int i = strlen(fileName) - 1; i >= 0; i--) {
        if (fileName[i] == '\\') {
            copySize = i + 1;
            break;
        }
    }
    strcpy_s(filePath, MAX_LINE_LENGTH, fileName);
    filePath[copySize] = '\0';

    return filePath;
}

// 전처리 전 헤더파일 추출
// #include "파일 이름" -> 파일 이름 추출
char* extractHeaderFile(char* buf) {
    char* context = NULL;
    char* token = strtok_s(buf, "\"", &context);

    while (token != NULL) {
        if (strstr(token, "#include") != NULL) {
            token = strtok_s(NULL, "\"", &context);
            continue;
        }
        else if (strtok_s(NULL, "\"", &context) != NULL && strstr(token, ".h")) return token;
        else return NULL;
    }
}

// 전처리 전 코드파일 추출
// 파일 경로 + 파일 이름 -> 파일 이름 추출
char* extractFileName(char* buf) {
    int copySize = 0;
    char fileName[MAX_LINE_LENGTH] = "";

    for (int i = strlen(buf) - 1; i >= 0; i--) {
        if (buf[i] == '\\') {
            copySize = i + 1;
            break;
        }
    }
    strcpy_s(fileName, buf + copySize);
    fileName[strlen(buf) - copySize] = '\0';

    return fileName;
}

// #define var_name val -> val_name 추출
char* extractVar_Name(char* buf) {
    char* context = NULL;

    if (strcmp(strtok_s(buf, " ", &context), "#define") == 0) {
        return strtok_s(NULL, " ", &context);
    }
    else {
        printf("Wrong Function Called.\nRefer to the #define statement\n\n");
        exit(1);
    }
}

// #define var_name val -> val 추출
char* extractVal(char* buf) {
    char* context = NULL;
    char token[MAX_LINE_LENGTH];

    if (strcmp(strtok_s(buf, " ", &context), "#define") == 0) {
        strtok_s(NULL, " ", &context);
        strcpy_s(token, MAX_LINE_LENGTH, strtok_s(NULL, " ", &context));
        if (token[strlen(token) - 1] == '\n') token[strlen(token) - 1] = '\0';
        return token;
    }
    else {
        printf("Wrong Function Called.\nRefer to the #define statement\n\n");
        exit(1);
    }
}

// define 변환 function
char* replaceVarToVal(char* buf, char* var, char* val) {
    char result[MAX_LINE_LENGTH];
    char* ptr = strstr(buf, var);

    if (ptr != NULL) {
        strcpy_s(result, MAX_LINE_LENGTH, buf);
        result[strlen(buf) - strlen(ptr)] = '\0';
        strcat_s(result, val);
        strcat_s(result, ptr + strlen(var));
    }

    return result;
}

// #include, #define 전처리 실행
void preprocessFile(char* filePath) {
    FILE* ppFile = NULL;
    FILE* tmpFile = NULL;
    char tmpFilePath[MAX_LINE_LENGTH];
    char buf[MAX_LINE_LENGTH];

    // 복사용 임시 파일 생성 tmpFile
    strcpy_s(tmpFilePath, MAX_LINE_LENGTH, extractFilePath(filePath));
    strcat_s(tmpFilePath, "tmp.txt");
    if (fopen_s(&tmpFile, tmpFilePath, "w+") != 0) {
        printf("File Not Found.\n");
    }

    if (fopen_s(&ppFile, filePath, "r+") == 0) {
        while (fgets(buf, MAX_LINE_LENGTH, ppFile) != NULL) {
            if (acceptInclude(buf)) {
                char headerPath[MAX_LINE_LENGTH] = "";
                char headerName[MAX_LINE_LENGTH] = "";
                char headerFile[MAX_LINE_LENGTH];

                // 전처리할 헤더파일의 전체 경로, 경로, 이름 설정
                strcpy_s(headerFile, extractHeaderFile(buf));
                if (strstr(headerFile, "\\") == NULL) {
                    strcpy_s(headerName, MAX_LINE_LENGTH, headerFile);
                    strcpy_s(headerFile, MAX_LINE_LENGTH, filePath);
                    strcat_s(headerFile, headerName);
                }
                else {
                    strcpy_s(headerPath, MAX_LINE_LENGTH, extractFilePath(headerFile));
                    strcpy_s(headerName, MAX_LINE_LENGTH, extractFileName(headerFile));
                }

                if (isIncluded(headerName) == false) {
                    push(headerName);

                    FILE* hFile = NULL;
                    if (fopen_s(&hFile, headerFile, "r") == 0) {
                        while (fgets(buf, MAX_LINE_LENGTH, hFile) != NULL) {
                            // 헤더파일의 입력을 상대경로로 입력 시 해당 파일의 경로를 기준으로 절대경로로 수정하여 설정
                            if (acceptInclude(buf) && strstr(buf, "\\") == NULL) {
                                char tmp[MAX_LINE_LENGTH];
                                char* context = NULL;

                                strcpy_s(tmp, MAX_LINE_LENGTH, strtok_s(buf, "\"", &context));
                                strcat_s(tmp, "\"");
                                strcat_s(tmp, headerPath);
                                strcat_s(tmp, MAX_LINE_LENGTH, strtok_s(NULL, "\"", &context));
                                strcat_s(tmp, "\"\n");

                                strcpy_s(buf, MAX_LINE_LENGTH, tmp);
                            }
                            fputs(buf, tmpFile);
                        }

                        fputs("\n", tmpFile);
                        while (fgets(buf, MAX_LINE_LENGTH, ppFile) != NULL) fputs(buf, tmpFile);

                        // ppFile(pathportFile)에 복사
                        fclose(ppFile);
                        rewind(tmpFile);
                        if (fopen_s(&ppFile, filePath, "w") == 0) {
                            while (fgets(buf, MAX_LINE_LENGTH, tmpFile) != NULL) fputs(buf, ppFile);
                        }

                        fclose(ppFile);
                        fclose(tmpFile);
                        remove(tmpFilePath);
                        preprocessFile(filePath);
                        return;
                    }
                }
            }
            else if (acceptDefine(buf)) {
                char var[MAX_LINE_LENGTH];
                char val[MAX_LINE_LENGTH];
                char tmp[MAX_LINE_LENGTH];

                // #define var val -> var, val 추출
                strcpy_s(tmp, MAX_LINE_LENGTH, buf);
                strcpy_s(var, MAX_LINE_LENGTH, extractVar_Name(tmp));
                strcpy_s(tmp, MAX_LINE_LENGTH, buf);
                strcpy_s(val, MAX_LINE_LENGTH, extractVal(tmp));

                // var -> val 대체 매크로
                while (fgets(buf, MAX_LINE_LENGTH, ppFile) != NULL) {
                    // 다른 #define문과 var 이름이 겹칠 경우 예외처리
                    if (acceptDefine(buf) && strstr(buf, var) != NULL) { 
                        fputs(buf, tmpFile);
                        while (fgets(buf, MAX_LINE_LENGTH, ppFile) != NULL) fputs(buf, tmpFile);
                        break;
                    }
                    else if (strstr(buf, var) != NULL) {
                        strcpy_s(buf, MAX_LINE_LENGTH, replaceVarToVal(buf, var, val));
                    }
                    fputs(buf, tmpFile);
                }

                // ppFile에 복사
                fclose(ppFile);
                rewind(tmpFile);
                if (fopen_s(&ppFile, filePath, "w") == 0) {
                    while (fgets(buf, MAX_LINE_LENGTH, tmpFile) != NULL) fputs(buf, ppFile);
                }

                fclose(ppFile);
                fclose(tmpFile);
                remove(tmpFilePath);
                preprocessFile(filePath);
                return;
            }
            else if (remove_comments(buf)) {
                char* comment_start = strstr(buf, "//");
                if (comment_start != NULL) {
                    *comment_start = '\0';  // 주석 부분을 문자열 끝으로 설정
                }
                return;
            }
            else fputs(buf, tmpFile);
        }
    }
    // ppFile에 복사
    fclose(ppFile);
    rewind(tmpFile);
    if (fopen_s(&ppFile, filePath, "w") == 0) {
        while (fgets(buf, MAX_LINE_LENGTH, tmpFile) != NULL) fputs(buf, ppFile);
    }

    fclose(ppFile);
    fclose(tmpFile);
    remove(tmpFilePath);
}

void preprocess_file(FILE *file, char *fileName, char *ppFileName) {

}

int main() {
    FILE* file = NULL;
    char fileName[MAX_LINE_LENGTH];
    char ppFileName[MAX_LINE_LENGTH];

    printf("전처리 할 파일의 경로를 입력하세요. (절대 경로): ");
    scanf_s("%s", fileName, MAX_LINE_LENGTH);

    if (fopen_s(&file, fileName, "r") == 0) {
        FILE* ppFile = NULL;
        char buf[MAX_LINE_LENGTH];
        char name[MAX_LINE_LENGTH];
        char fileType[MAX_LINE_LENGTH] = ".";
        char* context = NULL;

        // 파일명 + 파일타입 분리
        strcpy_s(name, MAX_LINE_LENGTH, strtok_s(extractFileName(fileName), ".", &context));
        strcat_s(fileType, strtok_s(NULL, ".", &context));

        // 결과 파일 생성 및 복사
        strcpy_s(ppFileName, MAX_LINE_LENGTH, extractFilePath(fileName));
        strcat_s(ppFileName, "_pp");
        strcat_s(ppFileName, name);
        strcat_s(ppFileName, fileType);
        if (fopen_s(&ppFile, ppFileName, "w") == 0) {
            while (fgets(buf, MAX_LINE_LENGTH, file) != NULL) {
                // 헤더파일이 상대경로일시 해당 파일의 경로를 기준으로 절대경로로 수정하여 복사
                if (acceptInclude(buf) && strstr(buf, "\\") == NULL) {
                    char tmp[MAX_LINE_LENGTH];
                    context = NULL;

                    strcpy_s(tmp, MAX_LINE_LENGTH, strtok_s(buf, "\"", &context));
                    strcat_s(tmp, "\"");
                    strcat_s(tmp, MAX_LINE_LENGTH, extractFilePath(ppFileName));
                    strcat_s(tmp, MAX_LINE_LENGTH, strtok_s(NULL, "\"", &context));
                    strcat_s(tmp, "\"\n");

                    strcpy_s(buf, MAX_LINE_LENGTH, tmp);
                }
                fputs(buf, ppFile);
            }
            fclose(file);
            fclose(ppFile);

            // 파일 전처리
            preprocessFile(ppFileName);
            clearStack();
        }
        else {
            printf("File Not Opened.\n");
            exit(1);
        }
    }
    else printf("File Not Found.\n");

    return 0;
}


