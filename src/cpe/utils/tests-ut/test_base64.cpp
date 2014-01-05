#include "cpe/utils/stream_mem.h"
#include "cpe/utils/base64.h"
#include "cpe/utils/tests-env/test-fixture.hpp"

class Base64Test : public testenv::fixture<> {
public:
    const char * encode(const char * input) {
        read_stream_mem is = CPE_READ_STREAM_MEM_INITIALIZER(input, strlen(input));

        void * output_buf = t_tmp_alloc(256);
        write_stream_mem os = CPE_WRITE_STREAM_MEM_INITIALIZER(output_buf, 256);

        cpe_base64_encode((write_stream_t)&os, (read_stream_t)&is);

        return (const char *)output_buf;
    }

    const char * decode(const char * input) {
        read_stream_mem is = CPE_READ_STREAM_MEM_INITIALIZER(input, strlen(input));

        void * output_buf = t_tmp_alloc(256);
        write_stream_mem os = CPE_WRITE_STREAM_MEM_INITIALIZER(output_buf, 256);

        cpe_base64_decode((write_stream_t)&os, (read_stream_t)&is);

        return (const char *)output_buf;
    }
};

TEST_F(Base64Test, encode) {
    EXPECT_STREQ(
        "MTIzNDU2"
        ,
        encode("123456"));
}

TEST_F(Base64Test, decode) {
    EXPECT_STREQ(
        "123456"
        ,
        encode("MTIzNDU2"));
}

