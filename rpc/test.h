#pragma once

#define TEST_ZMQ
#define TEST_MPACK

void test_mpack ();
void main1 ();
void *start_rpcs_thread (void *ctx);
void *rpcs_thread_start (void *ctx);
