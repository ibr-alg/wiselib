#!/bin/bash

stdbuf -oL -eL ./app < ./shawn.novis.conf|~/bin/timestamp |tee shawn.log

