#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/base64.h"

static const char table64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t cpe_base64_encode(write_stream_t output, read_stream_t input) {
    char buf[128];
    int wlen = 0;
    int buf_len = 0;

    for(buf_len = stream_read(input, buf, sizeof(buf));
        buf_len > 0;
        buf_len = stream_read(input, buf, sizeof(buf)))
    {
        char * rp = buf;
        while(buf_len > 0) {
            stream_putc(output, table64[(rp[0] >> 2 ) & 0x3F]);  //右移两位，与00111111是防止溢出，自加  

            if(buf_len > 2) { //够3个字符  
                stream_putc(output, table64[((rp[0] & 3) << 4) | (rp[1] >> 4)]);  
                stream_putc(output, table64[((rp[1] & 0xF) << 2) | (rp[2] >> 6)]);
                stream_putc(output, table64[rp[2] & 0x3F]);
            }
            else {
                switch(buf_len) {    //追加“=”  
                case 1:  
                    stream_putc(output, table64[(rp[0] & 3) << 4 ]);
                    stream_putc(output, '=');
                    stream_putc(output, '=');
                    break;  
                case 2:  
                    stream_putc(output, table64[((rp[0] & 3) << 4) | (rp[1] >> 4)]);
                    stream_putc(output, table64[((rp[1] & 0x0F) << 2) | (rp[2] >> 6)]);
                    stream_putc(output, '=');
                    break;  
                }
            }
        }

        rp += 3;  
        buf_len -= 3;  
        wlen +=4;  
    }

    return wlen;  
}  

char cpe_base64_char_to_index(char c) {
    if((c >= 'A') && (c <= 'Z')) return c - 'A';
    if((c >= 'a') && (c <= 'z')) return c - 'a' + 26;
    if((c >= '0') && (c <= '9')) return c - '0' + 52;
    if(c == '+') return 62;
    if(c == '/') return 63;  
    if(c == '=') return 0;  
    return 0;  
}

size_t cpe_base64_decode(write_stream_t output, read_stream_t input) {
    char buf[128];
    int wlen = 0;
    int buf_len;
    char lpCode[4];

    for(buf_len = stream_read(input, buf, sizeof(buf));
        buf_len > 0;
        buf_len += stream_read(input, buf + buf_len, sizeof(buf) - buf_len))
    {
        char * rp = buf;
        while(buf_len > 2) {      /*不足三个字符，忽略 */
            lpCode[0] = cpe_base64_char_to_index(rp[0]);
            lpCode[1] = cpe_base64_char_to_index(rp[1]);
            lpCode[2] = cpe_base64_char_to_index(rp[2]);
            lpCode[3] = cpe_base64_char_to_index(rp[3]);

            stream_putc(output, (lpCode[0] << 2) | (lpCode[1] >> 4));
            stream_putc(output, (lpCode[1] << 4) | (lpCode[2] >> 2));
            stream_putc(output, (lpCode[2] << 6) | (lpCode[3]));
  
            rp += 4;  
            buf_len -= 4;  
            wlen += 3;  
        }

        if (buf_len > 0) {
            memmove(buf, rp, buf_len);
        }
    }

    return wlen;  
}
