build:
	gcc spam_detector.c -o spam_detector -lm
run:
	./spam_detector
clean:
	rm -rf spam_detector
