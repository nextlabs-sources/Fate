#ifndef NLQUENCHHASH_H
#define NLQUENCHHASH_H

#include <string>
std::string generateData(const std::string &sharedSecret);
DWORD validateData (const std::string &inputData, time_t currentTime, const std::string &secret);

#define QUENCH_NON_SECRET_1  "463c32b601da2f5362b42000550beb28e989dfef1944c3b4f39e8349baa7917a0f7dfa66cef86abc"
#define QUENCH_NON_SECRET_2  "6f32d2ee2868e986ae14492b31cb443e7b300f13937c97a7f9b1fdeabd1ac3f1ad741e6488cc6a50"
#define QUENCH_SHARED_SECRET "a49fe738278d1ecd787675b740373537c5cde38a44983529687440243539e6f77d53a9132fa8b754"
#define QUENCH_NON_SECRET_3  "34ad535e6072cef12031b44c42336989fc391948ca487c44817b255a30eed62a17730fc01f1dd345"


#endif
