cmake_minimum_required(VERSION 3.10~3.22)
project(llhttp VERSION 1.0 LANGUAGES C)

aux_source_directory(. llhttp_source)
message("llhttp_source : ${llhttp_source}")

add_library(llhttp STATIC ${llhttp_source})