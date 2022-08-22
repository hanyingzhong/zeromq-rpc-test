#include <assert.h>
#include <iostream>
#include <tchar.h>
#include "mpack.h"
#include "rpc_comm.h"
#include "test.h"

void test_rpc_request ();

//#pragma comment(lib, "libzmq.lib")
//printf("%.*s\n", str_len, str);
void parse_request (char *data, size_t size);
void my_test_hash_set ();

void main2 ()
{
    test_mpack ();
    parse_request ("\x92\xa"
                   "3add\x82"
                   "\xa"
                   "1a\xaa"
                   "ddddddddd\x00\xa""1b\x0f",
                   100);
}

//#undef TEST_ZMQ
void main (int argc, TCHAR *argv[])
{
    rpc_hash_mgr_init ();
    my_test_hash_set ();
#ifdef TEST_ZMQ
    main1 ();
#endif
    main2 ();
    //test_rpc_request ();
}
