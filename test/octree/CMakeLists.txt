cmake_minimum_required (VERSION 3.8)
project(nori)


add_executable(Test

  ../../src/octree.h
  ../../include/nori/bbox.h

  octree_test.cpp
  ../../src/octree.cpp
)
