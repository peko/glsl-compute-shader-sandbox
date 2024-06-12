#!/bin/bash

# nvcc -O1 -arch=sm_75 -use_fast_math ECL-BH_45.cu -o ecl-bh
nvcc -O1 -arch=sm_50 -use_fast_math ECL-BH_45.cu -o ecl-bh

