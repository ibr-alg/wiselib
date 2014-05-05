#!/bin/bash

grep TOKEN $1|cut -c 23-31|sort|uniq -c|sort -n

