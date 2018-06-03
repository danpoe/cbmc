#!/bin/bash

NUM1=1000000
NUM2=10000000

function measure()
{
  echo '>>> Memory test'
  echo 'Unordered map'
  /usr/bin/time ./sharing_map_perf $1 u m
  echo 'Map'
  /usr/bin/time ./sharing_map_perf $1 m m
  echo 'Sharing'
  /usr/bin/time ./sharing_map_perf $1 s m

  echo '>>> Insert runtime'
  echo 'Unordered map repeated'
  ./sharing_map_perf $1 u i r
  echo 'Unordered map not repeated'
  ./sharing_map_perf $1 u i
  echo 'Map repeated'
  ./sharing_map_perf $1 m i r
  echo 'Map not repeated'
  ./sharing_map_perf $1 m i
  echo 'Sharing repeated'
  ./sharing_map_perf $1 s i r
  echo 'Sharing not repeated'
  ./sharing_map_perf $1 s i

  echo '>>> Find runtime'
  echo 'Unordered map'
  ./sharing_map_perf $1 u f
  echo 'Map'
  ./sharing_map_perf $1 m f
  echo 'Sharing'
  ./sharing_map_perf $1 s f
}

measure $NUM1
measure $NUM2

