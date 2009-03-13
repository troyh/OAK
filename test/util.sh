#!/bin/bash

function TEST
{
	if [ "$URL" == "" ]; then
		echo "URL not specified.";
		return;
	fi
	POST_DATA=$1
	EXPECTED_CODE=$2;
	
	read VER CODE MSG <<<`curl -D - --silent -d "$POST_DATA" $URL | head -n 1`;

	# echo "POST_DATA=$POST_DATA";
	# echo "EXPECTED_CODE=$EXPECTED_CODE";
	# echo "CODE=$CODE"
	
	if [ "$CODE" != "$EXPECTED_CODE" ]; then
		echo "$POST_DATA: Expected $EXPECTED_CODE, got $CODE $MSG";
	else
		echo "OK";
	fi
	
}
