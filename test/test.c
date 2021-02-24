int test1(){0;}
int test2(){42;}
int test3(){4+3-5;}
int test4(){9-10+4;}
int test5(){ 4 + 3   - 5;}
int test6(){9 -  10 +4;}
int test7(){5+6*7;}
int test8(){5*(9-6);}
int test9(){(3+5)/2;}
int test10(){-8+10;}
int test11(){+(-8)-(-10);}
int test12(){4+1>3;}
int test13(){4*1>=4;}
int test14(){4*1<+3;}
int test15(){4*1 <= -(-4);}

int count;
int assert(int answer,int res){
    if(answer == res){
        print("expect: %d,actual: %d, test%d is pass\n",answer,res,count);
    }else{
        print("expect: %d,actual: %d, test%d is fail\n",answer,res,count);
        doexit(1);
    }
    count=count+1;
}

int main(){
    count=1;
    assert(0,test1());
    assert(42,test2());
    assert(2,test3());
    assert(3,test4());
    assert(2,test5());
    assert(3,test6());
    assert(47,test7());
    assert(15,test8());
    assert(4,test9());
    assert(2,test10());
    assert(2,test11());
    assert(1,test12());
    assert(1,test13());
    assert(0,test14());
    assert(1,test15());
}