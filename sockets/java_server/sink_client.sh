#!/bin/bash

nohup ./sink_client_static localhost 1000 &> /tmp/sink.output &
