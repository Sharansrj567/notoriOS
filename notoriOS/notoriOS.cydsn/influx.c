#include "influx.h"  
#include "notoriOS.h"
                        
unsigned int construct_influx_write_body(char* body, char *node_id){
   
    body[0] = '\0';
 
    //construct influx body
    for(uint16 i = 0;i<sizeOfDataStack();i++){
        //note that influx uses a timestamp in nano-seconds, so we have to add 9 zeros
        snprintf(body,sizeof(http_body), "%s%s,node_id=%s value=%s %ld000000000\n", body, data[i].key, node_id, data[i].value, data[i].timeStamp);
    }
    
    unsigned int request_len = strlen(body);
   

    return request_len;
}

void construct_influx_route(char* route, char* base, char* user, char* pass, char* database){
    route[0] = '\0';
    snprintf(route,sizeof(http_route), "%s%s?u=%s&p=%s&db=%s", route, base, user, pass, database); 
}

test_t influx_test(){
    test_t test;
    test.status = 0u;
    snprintf(test.test_name,sizeof(test.test_name),"INFLUX");
    
    //we'll create a few fake test, generate a string and compare it to a known value
    char *expected_string = "test_reading_1,node_id=XYZ10, value=000 12345\ntest_reading_2,node_id=XYZ10, value=111 67890\r\n";
    
    Clear_Data_Stack();
    pushData("test_reading_1", "000", 12345);
    pushData("test_reading_2", "111", 67890);
    char test_write_body[200];
    construct_influx_write_body(test_write_body, "XYZ10");
    
    //will return zero if strings are identical,
    int compare = strcmp(test_write_body,expected_string);
    
    if(compare == 0){
        test.status = 1u;
        snprintf(test.reason,sizeof(test.reason),"Influx body generated correctly.");
    }else{
        snprintf(test.reason,sizeof(test.reason),"Influx body could not be generated correctly.");
    }
    
    Clear_Data_Stack();
    
    return test;
}