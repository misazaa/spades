#!/bin/sh

RUNNER_PARAM_SOAP_BASE/SOAPdenovo-127mer all -K RUNNER_PARAM_K -F -R -E -w -u -s RUNNER_PARAM_SOAP_CONFIG -o asm -p RUNNER_PARAM_THREADS
RUNNER_PARAM_SOAP_BASE/GapCloser -b RUNNER_PARAM_SOAP_CONFIG -a asm.scafSeq -o asm.new.scafSeq -t RUNNER_PARAM_THREADS