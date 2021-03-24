extern int foo();
extern int hoge(int x, int y);
extern int hogege(int a, int b, int c, int d, int e, int f);
extern int hogehoge(int a, int b, int c, int d, int e, int f, int g);
extern int print(const char *fmt, ...);
extern int print_num(int x);
extern int test(char *fmt);
extern int fact(int x);
extern void alloc4(int **p, int a, int b, int c, int d);
extern void doexit(int x);

#define TEST_BEGIN() int ans, act;
#define TEST(answer, test)                                                  \
    ans = answer;                                                           \
    act = test();                                                           \
    if (ans == act)                                                         \
        print("test:%-20s is pass! :: answer: %-5d, actual: %-5d\n", #test, \
              ans, act);                                                    \
    else {                                                                  \
        print("test:%-10s is failed... :: answer: %-5d, actual: %-5d\n",    \
              #test, ans, act);                                             \
        doexit(1);                                                          \
    }
#define TEST_END()

int test1() { 0; }
int test2() { 42; }
int test3() { 4 + 3 - 5; }
int test4() { 9 - 10 + 4; }
int test5() { 4 + 3 - 5; }
int test6() { 9 - 10 + 4; }
int test7() { 5 + 6 * 7; }
int test8() { 5 * (9 - 6); }
int test9() { (3 + 5) / 2; }
int test10() { -8 + 10; }
int test11() { +(-8) - (-10); }
int test12() { 4 + 1 > 3; }
int test13() { 4 * 1 >= 4; }
int test14() { 4 * 1 < +3; }
int test15() { 4 * 1 <= -(-4); }
int test16() { (4 * 3) - 1 != 5 * 2; }
int test17() {
    int x;
    x = 3;
    x * 4 + 3;
}
int test18() {
    int x = 0;
    return ++x;
}
int test19() {
    int x = 1;
    return --x;
}
int test20() {
    int x = 0;
    return x++;
}
int test21() {
    int x = 1;
    return x--;
}
int test22() {
    int x = 0;
    for (int i = 0; i < 5; i++) {
        x = x + i;
    }
    return x;
}
int test23() {
    char *x = "ahoaho";
    x++;
    return x[0] - x[2];
}
int test24() {
    int x[5];
    for (int i = 0; i < 5; i++) {
        x[i] = i;
    }
    int *y = x + 2;
    print_num(*y);
    y++;
    print_num(*y);
    y--;
    print_num(*y);
    y--;
    return *y;
}
int test25() {
    int x = 9;
    x += 3;
    return x;
}
int test26() {
    int x = 9;
    x -= 4;
    return x;
}
int test27() {
    int x = 9;
    x *= 2;
    return x;
}
int test28() {
    int x = 9;
    x /= 3;
    return x;
}
int test29() {
    int a;
    a = 4;
    int d;
    d = 3 * a;
    +(a + d) * 3;
}
int test30() {
    int abc;
    abc = 4;
}
int test31() {
    int abc;
    int abcd;
    abc = 5;
    abcd = abc / 5;
}
int test32() {
    int x;
    x = 6;
    /*x=9;
    x++;
    */
    return x;
}
int test33() {
    int x;
    x = 6;
    // x=9;
    return x;
}
int test34() {
    int aho;
    int aho_aho;
    aho = 3;
    aho_aho = aho * 2;
    return aho_aho * aho;
}
int test35() {
    int x_1;
    int x_2;
    x_1 = 100;
    x_2 = 2;
    return x_1 / x_2;
    return 0;
}
int test36() {
    int x;
    x = 9;
    if (x == 9) return 5;
    return 3;
}
int test37() {
    int x;
    x = 9;
    if (x != 9) return 5;
    return 3;
}
int test38() {
    int x;
    x = 9;
    if (x != 9)
        return 5;
    else
        return 3;
    return 1;
}
int test39() {
    int x;
    x = 9;
    if (x == 5)
        x = x * 3;
    else
        x = x * 4;
    return x;
}
int test40() {
    int x;
    x = 9;
    if (x == 3)
        if (x == 8)
            return 3;
        else
            return 5;
    else
        return 6;
}
int test41() {
    int x;
    x = 0;
    while (x < 5) x = x + 1;
    return x;
}
int test42() {
    int x;
    x = 0;
    while (x < 5)
        if (x < 4)
            x = x + 1;
        else
            x = x + 4;
    return x;
}
int test43() {
    int i;
    int x;
    i = 0;
    for (x = 0; x < 5; x = x + 1) i = i + x;
    return i;
}
int test44() {
    int i;
    int x;
    i = 0;
    for (x = 0; x < 5; x = x + 1)
        if (x == 3) i = i + x;
    return i;
}
int test45() { 3; }
int test46() {
    int x;
    {
        x = 3;
        x * 4;
    }
    return 5;
}
int test47() {
    int y_1;
    int y_2;
    int z;
    y_1 = 0;
    y_2 = 1;
    int x;
    for (x = 0; x < 3; x = x + 1) {
        z = y_1;
        y_1 = y_2;
        y_2 = z + y_2;
    }
    return y_2;
}
int test48() {
    int i;
    int x;
    for (i = 0; i < 5; i = i + 1) foo();
    x = 3;
}
int test49() {
    int x;
    int y;
    int z;
    x = 3;
    y = 5;
    z = hoge(x, y);
}
int test50() {
    int x;
    int y;
    int z;
    x = 1;
    y = 1;
    int i;
    for (i = 0; i < 5; i = i + 1) {
        z = hoge(x, y);
        x = y;
        y = z;
    }
    return y;
}
int test51() { hogege(1, 1, 1, 1, 1, 1); }
int test52() { hogehoge(1, 1, 1, 1, 1, 1, 1); }
int test53() {
    { 3; }
}
int add(int x, int y) {
    int z;
    z = x + y;
    return z;
}
int test54() {
    int x = 1;
    int y = 1;
    int z;
    z = add(x, y);
    return z;
}
int Fibonacci(int x, int y) {
    int z;
    z = x + y;
    return z;
}
int test55() {
    int x;
    int y;
    x = 1;
    y = 1;
    int i;
    int z;
    for (i = 0; i < 10; i = i + 1) {
        z = Fibonacci(x, y);
        x = y;
        y = z;
        print("%d\n", y);
    }
    return y;
}
int max(int x, int y) {
    if (x > y)
        return x;
    else
        return y;
}
int test56() {
    int x;
    int y;
    x = 6;
    y = 5;
    return max(x, y);
}
int huga(int x) { x = 3; }
int test57() {
    int x;
    x = 5;
    huga(x);
    return x;
}
int test58() { return fact(4); }
int rec(int x) {
    if (x == 0) return 0;
    return x + rec(x - 1);
}
int test59() { return rec(5); }
int Euclid(int x, int y) {
    int z;
    if (x > y) {
        z = x;
        x = y;
        y = z;
    }
    if (x == 0) return y;
    return Euclid(x, y - x);
}
int test60() { return Euclid(6, 222); }
int sum6(int a, int b, int c, int d, int e, int f) {
    return a + b + c + d + e + f;
}
int test61() { return sum6(1, 1, 1, 1, 1, 1); }
int test62() {
    int x;
    int *y;
    x = 5;
    y = &x;
    return *y;
}
int test63() {
    int x;
    int *y;
    y = &x;
    *y = 3;
    return x;
}
int test64() {
    int x;
    int y;
    int *z;
    x = 5;
    y = 9;
    z = &y + 1;
    return *z;
}
int test65() {
    int x;
    int y;
    *(&y + 1) = 3;
    return x;
}
int test66() {
    int x;
    x = 3;
    int *y;
    y = &x;
    return *y;
}
int sum7(int a, int b, int c, int d, int e, int f, int g) {
    return a + b + c + d + e + f + g;
}
int test67() { return sum7(1, 1, 1, 1, 1, 1, 1); }
int test68() {
    int *p;
    alloc4(&p, 1, 2, 4, 8);
    return *p;
}
int test69() {
    int *p;
    alloc4(&p, 1, 2, 4, 8);
    return *(p + 2);
}
int test70() {
    int *p;
    alloc4(&p, 1, 2, 4, 8);
    int *q;
    q = p + 2;
    *q;
    q = p + 3;
    return *q;
}
int test71() {
    int x;
    return sizeof(x);
}
int test72() {
    int *y;
    return sizeof(y);
}
int test73() { sizeof(5); }
int test74() {
    int *y;
    sizeof(y + 4);
}
int test75() { int x = 5; }
int test76() {
    int x[3];
    *x = 3;
}
int test77() {
    int x[3];
    *x = 5;
    return *x;
}
int test78() {
    int x[3];
    *x = 3;
    *(x + 1) = 10;
    *(x + 2) = 4;
    return *(x + 1);
}
int test79() {
    int x[3];
    x[0] = 3;
}
int test80() {
    int x[3];
    x[0] = 5;
    return x[0];
}
int test81() {
    int x[3];
    x[0] = 3;
    x[1] = 10;
    x[2] = 4;
    return x[1];
}
int test82() {
    int a[2];
    *a = 1;
    *(a + 1) = 2;
    int *p;
    p = a;
    return *p + *(p + 1);
}
int test83() {
    int x[3];
    0 [x] = 3;
    x[1] = 10;
    2 [x] = 4;
    return x[1];
}
int x;
int test84() {
    x = 5;
    return x;
}
int *y;
int test85() {
    x = 5;
    y = &x;
    return *y;
}
int test86() {
    char x[3];
    x[0] = -1;
    x[1] = 2;
    int y;
    y = 4;
    return x[0] + y;
}
char a[3];
void asrt(int x) { a[0] = x; }
int test87() {
    asrt(4);
    return a[0];
}
int test88() {
    char *x = "ahoaho";
    return x[0];
}
int test89() {
    char *x = "ahoaho";
    return x[1] - x[0];
}
int test90() {
    int x;
    x = 6;
    // x=9;
    return x;
}
int test91() {
    int x;
    x = 6;
    /*x=9;
    x++;
    */
    return x;
}

int *ahoaho(char *x) {
    print(x);
    return 3;
}
int test92() {
    int *x = ahoaho("aho");
    return x;
}
int test93() {
    int x = 10;
    {
        int x = 5;
        x += 4;
    }
    return x;
}
int sum(int a, int b) { return a + b; }
int test94() {
    int (*s)(int, int);
    s = sum;
    return (*s)(1, 2);
}
int test95() {
    int (*s)(int, int);
    s = sum;
    return s(1, 2);
}
int test96() {
    int x = 4;
    x = x << 1;
    return x;
}
int test97() {
    int x = 4;
    x = x >> 1;
    return x;
}
int test98() {
    int x = 4;
    int y = 2;
    x = x | y;
    return x;
}
int test99() {
    int x = 4;
    int y = 2;
    x = x & y;
    return x;
}
int test100() {
    int x = 4;
    int y = 4;
    x = x ^ y;
    return x;
}
int test101() {
    int x = 100;
    int y = 3;
    x = x % y;
    return x;
}
int test102() {
    int x = 4;
    x <<= 1;
    return x;
}
int test103() {
    int x = 4;
    x >>= 1;
    return x;
}
int test104() {
    int x = 4;
    int y = 2;
    x |= y;
    return x;
}
int test105() {
    int x = 4;
    int y = 2;
    x &= y;
    return x;
}
int test106() {
    int x = 4;
    int y = 4;
    x ^= y;
    return x;
}
int test107() {
    int x = 100;
    int y = 3;
    x %= y;
    return x;
}
int test108() {
    int x = 9;
    while (x == 8) {
        print("while!\n");
    }
    do {
        print("do while!\n");
        x++;
    } while (x == 8);
    return x;
}
int test109() {
    int x = 0;
    while (1) {
        if (x > 8) break;
        x++;
    }
    return x;
}
int test110() {
    int x = 0;
    for (; x < 10; x++) {
        if (x > 8) break;
    }
    return x;
}
int test111() {
    int x = 0;
    while (1) {
        if (x <= 8) {
            x++;
            continue;
        }
        break;
    }
    return x;
}
int test112() {
    int x = 0;
    for (;; x++) {
        if (x <= 8) continue;
        break;
    }
    return x;
}
int test113() {
    for (int x = 8; x < 20; x++) {
        while (x > 7) {
            x += 3;
            if (x <= 15) continue;
            break;
        }
        return x;
    }
}
int test114() {
    int x = 0;
    x = ~x;
    return x & 1;
}
int test115() {
    int x = 1;
    x = ~x;
    return x & 1;
}
int test116() {
    int x = 6;
    return x == 6 ? 4 : 5;
}
int test117() {
    int x = 6;
    return x != 6 ? 4 : 5;
}
int test118() {
    int x = 6, y = 7;
    return y;
}
int test119_a, test119_b, test119() {
    test119_a = 7;
    test119_b = 6;
    return test119_a;
}
#define aho 3
int test121() { return aho; }
#undef aho
#
#define aho x + y
int test122() {
    int x = 8, y = 7;
    return aho;
}
#undef aho
#
#define aho x + y
int test123() {
    int x = 8, y = 7;
    return aho;
}
#undef aho
#define ahoaho "test124:"
#define ahoho "clear\n"
int test124() {
    char *x = "***" ahoaho ahoho;
    print(x);
    return 3;
}
#undef ahoaho
#undef ahoho

#define aho() "test125:"
#define ahoaho(test) "clear\n"
int test125() {
    char *x = "***" aho() ahoaho(aha);
    print(x);
    return 3;
}
#undef aho
#undef ahoaho

#define aho() "test126:"
#define ahoaho(test) test
int test126() {
    char *x = "***" aho() ahoaho("clear\n");
    print(x);
    return 3;
}
#undef aho
#undef ahoaho

#define debug(value) print(#value "=%d\n", value)
int test127() {
    int x = 6;
    debug(x);
    return x;
}
#undef debug
#define AHO(a, b) a##b
#define AHOAHO 4
int test128() { return AHO(AHO, AHO); }
#undef AHO
#undef AHOAHO

extern int assert(int, int);
////////////////////////////////^テスト部
int Test() {
    TEST_BEGIN()
    TEST(0, test1)
    TEST(42, test2)
    TEST(2, test3)
    TEST(3, test4)
    TEST(2, test5)
    TEST(3, test6)
    TEST(47, test7)
    TEST(15, test8)
    TEST(4, test9)
    TEST(2, test10)
    TEST(2, test11)
    TEST(1, test12)
    TEST(1, test13)
    TEST(0, test14)
    TEST(1, test15)
    TEST(1, test16)
    TEST(15, test17)
    TEST(1, test18)
    TEST(0, test19)
    TEST(0, test20)
    TEST(1, test21)
    TEST(10, test22)
    TEST(7, test23)
    TEST(1, test24)
    TEST(12, test25)
    TEST(5, test26)
    TEST(18, test27)
    TEST(3, test28)
    TEST(48, test29)
    TEST(4, test30)
    TEST(1, test31)
    TEST(6, test32)
    TEST(6, test33)
    TEST(18, test34)
    TEST(50, test35)
    TEST(5, test36)
    TEST(3, test37)
    TEST(3, test38)
    TEST(36, test39)
    TEST(6, test40)
    TEST(5, test41)
    TEST(8, test42)
    TEST(10, test43)
    TEST(3, test44)
    TEST(3, test45)
    TEST(5, test46)
    TEST(3, test47)
    TEST(3, test48)
    TEST(8, test49)
    TEST(13, test50)
    TEST(6, test51)
    TEST(7, test52)
    TEST(3, test53)
    TEST(2, test54)
    TEST(144, test55)
    TEST(6, test56)
    TEST(5, test57)
    TEST(24, test58)
    TEST(15, test59)
    TEST(6, test60)
    TEST(6, test61)
    TEST(5, test62)
    TEST(3, test63)
    TEST(5, test64)
    TEST(3, test65)
    TEST(3, test66)
    TEST(7, test67)
    TEST(1, test68)
    TEST(4, test69)
    TEST(8, test70)
    TEST(4, test71)
    TEST(8, test72)
    TEST(4, test73)
    TEST(8, test74)
    TEST(5, test75)
    TEST(3, test76)
    TEST(5, test77)
    TEST(10, test78)
    TEST(3, test79)
    TEST(5, test80)
    TEST(10, test81)
    TEST(3, test82)
    TEST(10, test83)
    TEST(5, test84)
    TEST(5, test85)
    TEST(3, test86)
    TEST(4, test87)
    TEST(97, test88)
    TEST(7, test89)
    TEST(6, test90)
    TEST(6, test91)
    TEST(3, test92)
    TEST(10, test93)
    TEST(3, test94)
    TEST(3, test95)
    TEST(8, test96)
    TEST(2, test97)
    TEST(6, test98)
    TEST(0, test99)
    TEST(0, test100)
    TEST(1, test101)
    TEST(8, test102)
    TEST(2, test103)
    TEST(6, test104)
    TEST(0, test105)
    TEST(0, test106)
    TEST(1, test107)
    TEST(10, test108)
    TEST(9, test109)
    TEST(9, test110)
    TEST(9, test111)
    TEST(9, test112)
    TEST(17, test113)
    TEST(1, test114)
    TEST(0, test115)
    TEST(4, test116)
    TEST(5, test117)
    TEST(7, test118)
    TEST(7, test119)
    TEST(3, test121)
    TEST(15, test122)
    TEST(15, test123)
    TEST(3, test124)
    TEST(3, test125)
    TEST(3, test126)
    TEST(6, test127)

    TEST_END()
}
////////////////////////////////////////
int main() {
    Test();
    print("OK\n");
    return 0;
}