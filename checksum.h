//
// Created by Erez on 27/10/2023.
//

#ifndef DEFENSIVE_MAMAN_15_CHECKSUM_H
#define DEFENSIVE_MAMAN_15_CHECKSUM_H


#include <iostream>
#include <fstream>
#include <ostream>
#include <cstdio>
#include <vector>
#include <iterator>
#include <filesystem>
#include <string>

extern uint_fast32_t const crctab[8][256];

unsigned long memcrc(char * b, size_t n);
std::string readfile(std::string fname);
uint32_t readCrc(std::string fname);


#endif //DEFENSIVE_MAMAN_15_CHECKSUM_H
