#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_xml.h"
#include "WriteToXmlTest.hpp"

TEST_F(WriteToXmlTest, metalib_basic) {
    EXPECT_STREQ(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<metalib tagsetversion=\"1\" name=\"net\" version=\"10\">\n"
        "    <macro name=\"m1\" value=\"24\"/>\n"
        "    <struct name=\"PkgHead\" version=\"1\">\n"
        "        <entry name=\"a1\" type=\"int16\" id=\"12\" cname=\"a1.cname\" desc=\"a1.desc\"/>\n"
        "    </struct>\n"
        "</metalib>\n"
        ,
        writeToXml(
            "<metalib tagsetversion='1' name='net' version='10'>\n"
            "    <macro name='m1' value='24'/>\n"
            "    <struct name='PkgHead' version='1'>"
            "	     <entry name='a1' "
            "               desc='a1.desc'\n"
            "               cname='a1.cname'\n"
            "               type='int16'\n"
            "               id='12'/>"
            "    </struct>"
            "</metalib>"
            ));
}
