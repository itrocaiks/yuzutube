#ifndef UTF8_STR_H
#define UTF8_STR_H

/*------------------------------------------------------*/
/*-------------well, this one is better-----------------*/
/*-----------but there's no comments, bruh--------------*/
/*------------------------------------------------------*/

size_t utf8_strlen(char *str) {
    size_t length = 0;
    unsigned char c;

    while (*str) {
        c = (unsigned char)*str;

        if ((c & 0x80) == 0) {
            str += 1; 
        } else if ((c & 0xE0) == 0xC0) {
            str += 2;
        } else if ((c & 0xF0) == 0xE0) {
            str += 3; 
        } else if ((c & 0xF8) == 0xF0) {
            str += 4; 
        } else {
            str += 1; 
        }

        length++; 
    }

    return length;
}

unsigned char* utf8_at(char* str, size_t index) {
    size_t i = 0;
    size_t current_index = 0;

    while (str[i]) {
        unsigned char c = (unsigned char)str[i];

        size_t char_len = 1; 
        if ((c & 0x80) == 0) {
            char_len = 1; 
        } else if ((c & 0xE0) == 0xC0) {
            char_len = 2; 
        } else if ((c & 0xF0) == 0xE0) {
            char_len = 3;
        } else if ((c & 0xF8) == 0xF0) {
            char_len = 4;
        }

        if (current_index == index) {
            return (unsigned char*)&str[i];
        }

        i += char_len;
        current_index++;
    }

    return NULL; 
}

int utf8_char_length(unsigned char utf8_char) {
    if ((utf8_char & 0x80) == 0) {
        return 1;
    } else if ((utf8_char & 0xE0) == 0xC0) {
        return 2;
    } else if ((utf8_char & 0xF0) == 0xE0) {
        return 3;
    } else if ((utf8_char & 0xF8) == 0xF0) {
        return 4;
    } else {
        return -1; 
    }
}

// Функция для удаления первого символа UTF-8
void utf8_shift_left(char* str, int index) {
    int char_len = utf8_char_length((unsigned char)(str)[index]);
    if (char_len > 0) {
        memmove(str, str + char_len, strlen(str) - char_len + 1); 
    }
}


#endif
