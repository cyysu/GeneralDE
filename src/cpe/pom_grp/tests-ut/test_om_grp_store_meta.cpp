#include "OmGrpStoreTest.hpp" 

TEST_F(OmGrpStoreTest, basic) {
    install(
        "TestObj:\n"
        "  - entry1: { entry-type: normal, data-type: AttrGroup1 }\n"
        ,
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='AttrGroup1' version='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "</metalib>"
        ,
        "entry1"
        ,
        "a1") ;

    EXPECT_STREQ(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<metalib tagsetversion=\"0\" name=\"\" version=\"0\">\n"
        "    <struct name=\"AttrGroup1\" version=\"0\">\n"
        "        <entry name=\"a1\" type=\"uint32\" id=\"1\"/>\n"
        "    </struct>\n"
        "</metalib>\n"
        ,
        store_meta());
}

TEST_F(OmGrpStoreTest, multi_normal) {
    install(
        "TestObj:\n"
        "  - entry1: { entry-type: normal, data-type: AttrGroup1 }\n"
        "  - entry2: { entry-type: normal, data-type: AttrGroup2 }\n"
        ,
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='AttrGroup1' version='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "    <struct name='AttrGroup2' version='1'>"
        "	     <entry name='b1' type='uint32' id='2'/>"
        "    </struct>"
        "</metalib>"
        ,
        "entry1"
        ,
        "a1") ;

    EXPECT_STREQ(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<metalib tagsetversion=\"0\" name=\"\" version=\"0\">\n"
        "    <struct name=\"AttrGroup1\" version=\"0\">\n"
        "        <entry name=\"a1\" type=\"uint32\" id=\"1\"/>\n"
        "        <entry name=\"b1\" type=\"uint32\" id=\"2\"/>\n"
        "    </struct>\n"
        "</metalib>\n"
        ,
        store_meta());
}

TEST_F(OmGrpStoreTest, multi_ba) {
    install(
        "TestObj:\n"
        "  - entry1: { entry-type: normal, data-type: AttrGroup1 }\n"
        "  - entry2: { entry-type: ba, bit-capacity: 30, byte-per-page=2 }\n"
        ,
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='AttrGroup1' version='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "</metalib>"
        ,
        "entry1"
        ,
        "a1") ;

    EXPECT_STREQ(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<metalib tagsetversion=\"0\" name=\"\" version=\"0\">\n"
        "    <struct name=\"AttrGroup1\" version=\"0\">\n"
        "        <entry name=\"a1\" type=\"uint32\" id=\"1\"/>\n"
        "        <entry name=\"entry2\" type=\"uint8\" count=\"4\"/>\n"
        "    </struct>\n"
        "</metalib>\n"
        ,
        store_meta());
}

TEST_F(OmGrpStoreTest, multi_binary) {
    install(
        "TestObj:\n"
        "  - entry1: { entry-type: normal, data-type: AttrGroup1 }\n"
        "  - entry2: { entry-type: binary, capacity: 5 }\n"
        ,
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='AttrGroup1' version='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "</metalib>"
        ,
        "entry1"
        ,
        "a1") ;

    EXPECT_STREQ(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<metalib tagsetversion=\"0\" name=\"\" version=\"0\">\n"
        "    <struct name=\"AttrGroup1\" version=\"0\">\n"
        "        <entry name=\"a1\" type=\"uint32\" id=\"1\"/>\n"
        "        <entry name=\"entry2\" type=\"uint8\" count=\"5\"/>\n"
        "    </struct>\n"
        "</metalib>\n"
        ,
        store_meta());
}

TEST_F(OmGrpStoreTest, multi_list) {
    install(
        "TestObj:\n"
        "  - entry1: { entry-type: normal, data-type: AttrGroup1 }\n"
        "  - entry2: { entry-type: list, data-type: AttrGroup2, group-count: 3, capacity: 5 }\n"
        ,
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='AttrGroup1' version='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "    <struct name='AttrGroup2' version='1'>"
        "	     <entry name='b1' type='uint32' id='2'/>"
        "    </struct>"
        "</metalib>"
        ,
        "entry1"
        ,
        "a1") ;

    EXPECT_STREQ(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<metalib tagsetversion=\"0\" name=\"\" version=\"0\">\n"
        "    <struct name=\"AttrGroup1\" version=\"0\">\n"
        "        <entry name=\"a1\" type=\"uint32\" id=\"1\"/>\n"
        "    </struct>\n"
        "    <struct name=\"AttrGroup2\" version=\"0\">\n"
        "        <entry name=\"b1\" type=\"uint32\" id=\"2\"/>\n"
        "    </struct>\n"
        "</metalib>\n"
        ,
        store_meta());
}

TEST_F(OmGrpStoreTest, multi_list_standalone) {
    install(
        "TestObj:\n"
        "  - entry1: { entry-type: normal, data-type: AttrGroup1 }\n"
        "  - entry2: { entry-type: list, data-type: AttrGroup2, group-count: 3, capacity: 5, standalone: 1 }\n"
        ,
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='AttrGroup1' version='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "    <struct name='AttrGroup2' version='1'>"
        "	     <entry name='b1' type='uint32' id='2'/>"
        "    </struct>"
        "</metalib>"
        ,
        "entry1"
        ,
        "a1") ;

    EXPECT_STREQ(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<metalib tagsetversion=\"0\" name=\"\" version=\"0\">\n"
        "    <struct name=\"AttrGroup1\" version=\"0\">\n"
        "        <entry name=\"a1\" type=\"uint32\" id=\"1\"/>\n"
        "    </struct>\n"
        "    <struct name=\"AttrGroup2\" version=\"0\">\n"
        "        <entry name=\"a1\" type=\"uint32\" id=\"1\"/>\n"
        "        <entry name=\"b1\" type=\"uint32\" id=\"2\"/>\n"
        "    </struct>\n"
        "</metalib>\n"
        ,
        store_meta());
}

