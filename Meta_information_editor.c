#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* subString(char* str, int n) ///функция выделения подстроки начиная с n-ного символа
{
    char* res = (char*) malloc (sizeof(char) * (strlen(str) - n + 1)); ///выделяем память под строку
    res[strlen(str) - n] = '\0';
    for(int i = 0; i < strlen(str) - n; i++)
        res[i] = str[i + n];
    return res;
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("There is nothing to do\n");
        return 0;
    }

    char* path = (char*) malloc (sizeof(char) * strlen(argv[1]) - 9); ///выделяем память под строку
    path[strlen(argv[1]) - 8] = '\0'; ///в конец итоговой строки ставим символ конца строки
    for(int i = 0; i < strlen(argv[1]) - 9; i++)
        path[i] = argv[1][i + 11];    /// копируем символы

    FILE* stream = fopen(path, "rb");  ///Открыли файл
    if(stream == NULL)                 ///Если не получилось то выходим
    {
        printf("File with that name does not exists\n");
        free(path);
        return 0;
    }

    ///читаем МАРКЕР ЗАГОЛОВОЧНОГО ТЕГА ID3
    int n = 3;
    char* check_id3 = (char*) malloc (sizeof(char) * (n + 1)); ///выделяем память под строку
    check_id3[n] = '\0';  ///в конец итоговой строки ставим символ конца строки
    for(int i = 0; i < n; i++)
        check_id3[i] = getc(stream); ///читаем символы из файла, при этом указатель на текущий символ перемещается дальше

    if(strcmp(check_id3, "ID3"))            ///Если не id3 то выходим, strcmp выдаёт 0 если строки совпадают
    {
        free(path);
        free(check_id3);
        printf("There is no ID3 MetaData\n");
        return 0;
    }

    fseek(stream, 3, SEEK_CUR);           ///переставляем курсор на 3 байта вперед, пропускаем подверсию, флаги (курсор стоял на конце марикера т.к. уже его считали)
    n = 4; ///столько занимает места информация о длинне

    char* res = (char*) malloc (sizeof(char) * n); ///изначально в символьном виде
    int FrameSize = 0;
    for(int i = 0; i < n; i++)
    {
        res[i] = getc(stream);
        FrameSize = FrameSize << 7; ///так как на размер выдаётся 4 байта, а чтение начинается с конца числа 7-ой бит всегда = 0, то умножаем на 128 для корректного считывания
        FrameSize |= res[i]; ///побитовое сложение с результатом (добавление очередной части числа)
    }

    if(argc < 3)                          ///если не передали указание к действию то выходим
    {
        printf("Too few commands for action...\n");
        return 0;
    }

    if(!strcmp(argv[2], "--show"))        ///если команда show то начинаем выводить
    {
        while(ftell(stream) < FrameSize - 10) ///читаем файл пока не придем к концу, -10 байтов тк мы уже считали заголовок
        {
            ///ftell возвращает текущую поззицию указателя в потоке в файле чтения

            n = 4;
            char* frame_name = (char*) malloc (sizeof(char) * (n + 1)); ///выделяем память под строку
            frame_name[n] = '\0';  ///в конец итоговой строки ставим символ конца строки
            for(int i = 0; i < n; i++)
                frame_name[i] = getc(stream); ///читаем символы из файла, при этом указатель на текущий символ перемещается дальше

            char* res2 = (char*) malloc (sizeof(char) * n); ///изначально в символьном виде
            int frame_len = 0;
            for(int i = 0; i < n; i++)
            {
                res2[i] = getc(stream);
                frame_len = frame_len<< 7; ///так как на размер выдаётся 4 байта, а чтение начинается с конца числа 7-ой бит всегда = 0, то умножаем на 128 для корректного считывания
                frame_len |= res2[i]; ///побитовое сложение с результатом (добавление очередной части числа)
            }

            fseek(stream, 2, SEEK_CUR); ///перемещаем указатель на 2 байта вперед тк не читаем флаги
            if(frame_name[0] == 'T') ///если название фрейма начинается с "T" то это тэг и его выводим
            {
                n = frame_len;
                char* str = (char*) malloc (sizeof(char) * (n + 1)); ///выделяем память под строку
                str[n] = '\0';  ///в конец итоговой строки ставим символ конца строки
                for(int i = 0; i < n; i++)
                    str[i] = getc(stream); ///читаем символы из файла, при этом указатель на текущий символ перемещается дальше

                str[0] = ' ';
                printf("%s -> %s\n", frame_name, str);
            }
            else ///если не тэг то пропускаем и переходим к сл фрейму
            {
                free(frame_name);
                fseek(stream, frame_len, SEEK_CUR);
            }
        }
        return 0;
    }

    if(!strncmp(argv[2], "--get=", 6)) ///если необходимо получить значение определенного тэга
    {
        int check = 0; ///флаг проверки что тэг найден
        char* tag = subString(argv[2], 6); ///выделяем из аргумента нужную подстроку с именем тэга
        while(ftell(stream) < FrameSize - 10) ///пробегаем по всей мета-информации (-10 т.к. заголовок)
        {
            n = 4;
            char* frame_name = (char*) malloc (sizeof(char) * (n + 1)); ///выделяем память под строку
            frame_name[n] = '\0';  ///в конец итоговой строки ставим символ конца строки
            for(int i = 0; i < n; i++)
                frame_name[i] = getc(stream); ///читаем символы из файла, при этом указатель на текущий символ перемещается дальше

            n = 4;
            char* res = (char*) malloc (sizeof(char) * n); ///изначально в символьном виде
            int frame_len = 0;
            for(int i = 0; i < n; i++)
            {
                res[i] = getc(stream);
                frame_len = frame_len << 7; ///так как на размер выдаётся 4 байта, а чтение начинается с конца числа 7-ой бит всегда = 0, то умножаем на 128 для корректного считывания
                frame_len |= res[i]; ///побитовое сложение с результатом (добавление очередной части числа)
            }

            fseek(stream, 2, SEEK_CUR);  ///перемещаем указатель на 2 байта вперед тк не читаем флаги
            if(!strcmp(frame_name, tag)) ///если это фрейм с данным тэгом, то выводим его и возвращаем 1
            {

                n = frame_len;
                char* str = (char*) malloc (sizeof(char) * (n + 1)); ///выделяем память под строку
                str[n] = '\0';  ///в конец итоговой строки ставим символ конца строки
                for(int i = 0; i < n; i++)
                    str[i] = getc(stream); ///читаем символы из файла, при этом указатель на текущий символ перемещается дальше

                str[0] = ' ';
                printf("%s tag ->%s\n", frame_name, str);
                check = 1;
            }
            else ///иначе возвращаем 0
            {
                free(frame_name);
                fseek(stream, frame_len, SEEK_CUR); ///переставляем курсор на следующий фрейм
                check = 0;
            }
            //printf("%i\n",check);
            if(check == 1) ///если нашли, то выходим
            {
                free(tag);
                return 0;
            }
        }
        free(tag);///если дошли до сюда, то нужного фрейма нет
        printf("Frame with that tag does not exists\n");
        return 0;
    }

    if(!strncmp(argv[2], "--set=", 6)) ///если необходимо записать фрейм
    {
        char* tag = subString(argv[2], 6); ///берем значение нужного тэга
        char* value = subString(argv[3], 8); ///считываем значение которое необходимо установить
        int check = 0; /// флаг того, что нужный фрейм найден

        while(ftell(stream) < FrameSize - 10) ///перебираем всю мета-дату
        {

            n = 4;
            char* frame_name = (char*) malloc (sizeof(char) * (n + 1)); ///выделяем память под строку
            frame_name[n] = '\0';  ///в конец итоговой строки ставим символ конца строки
            for(int i = 0; i < n; i++)
                frame_name[i] = getc(stream); ///читаем символы из файла, при этом указатель на текущий символ перемещается дальше

            int frame_len = 0; ///читаем длинну фрейма (4 байта)
            n = 4;
            char* res3 = (char*) malloc (sizeof(char) * n); ///изначально в символьном виде
            for(int i = 0; i < n; i++)
            {
                res3[i] = getc(stream);
                frame_len = frame_len << 7; ///так как на размер выдаётся 4 байта, а чтение начинается с конца числа 7-ой бит всегда = 0, то умножаем на 128 для корректного считывания
                frame_len |= res3[i]; ///побитовое сложение с результатом (добавление очередной части числа)
            }

            fseek(stream, 2, SEEK_CUR);  ///перемещаем указатель на 2 байта вперед тк не читаем флаги
            if(!strcmp(frame_name, tag))  ///если нашли фрейм с таким тэгом то возвращаем его длину
            {
                check = frame_len;
            }
            else ///если тэг другой то возвращаем 0
            {
                free(frame_name);
                fseek(stream, frame_len, SEEK_CUR); ///перемещаем указатель на сл фрейм
                check = 0;
            }

            if(check)
                break;
        }
        if(ftell(stream) >= FrameSize - 10) ///если найти фрейм не полуичлось то выходим
        {
            free(tag);
            printf("Frame with that tag does not exists\n");
            return 0;
        }

        int pos = ftell(stream) - 6; ///Уже считали его тег + длину -> 6 байт

        fseek(stream, 0, SEEK_SET); ///перемещаем указатель в начало файла чтобы записать новую длинну мета-информации

        FILE* edit = fopen("data.mp3", "wb"); ///открываем файл на запись
        for(int i = 0; i < 6; i++) ///записываем значение тэга
        {
            char c = getc(stream);
            putc(c, edit);
        }

        FrameSize = FrameSize - check + strlen(value); ///вычисляем новую длину
        ///записываем новую длину

        char* buf = (char*) malloc (sizeof(char) * 4);
        for(int i = 0; i < 4; i++)
        {
            buf[i] = (FrameSize >> ((3 - i)*7)) & 0xFF;
            buf[i] = buf[i] & 0x7F;
            putc(buf[i], edit);
        }

        fseek(stream, ftell(edit), SEEK_SET); ///опять к началу файла и идем пока не найдем нужный фрейм
        while(ftell(edit) != pos) ///перебираем пока не придем к нужному фрейму
        {
            char c = getc(stream);
            putc(c, edit);
        }


        char* buf5 = (char*) malloc (sizeof(char) * 4);
        for(int i = 0; i < 4; i++)
        {
            buf5[i] = ((strlen(value) + 1) >> ((3 - i)*7)) & 0xFF;
            buf5[i] = buf5[i] & 0x7F;
            putc(buf5[i], edit);
        }

        putc(0x00, edit);
        putc(0x00, edit);
        putc(0x00, edit);

        for(int i = 0; i < strlen(value); i++)
            putc(value[i] & 0xFF, edit);

        fseek(stream, 4 + 2 + check, SEEK_CUR);

        while(1)
        {
            char c = getc(stream);
            if(c == EOF)
                if(feof(stream) != 0)
                    break;
            putc(c, edit);
        }

        fclose(stream);
        fclose(edit);

//        remove(path);
//        rename("data.mp3", path);

        free(value);
        free(tag);
        free(path);
    }
    return 0;
}
