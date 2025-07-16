# Target to build the iosched program
iosched: iosched.cpp
	# Use g++ to compile with debugging info
	g++ -g iosched.cpp -o iosched

# Clean target to remove the executable and backup files
clean:
	# Remove the iosched executable and backup files
	rm -f iosched *~
