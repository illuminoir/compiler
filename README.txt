Instructions on how to run the compiler in doc/doc.pdf

Known issues:
	Pointers not totally functionnal.
	The assignment between pointers works, as long as the left and right values are in the same scope.
	If not, the value of a pointer from another scope will not be retrieved correctly.
	Functions with more than 6 parameters not implemented
	reade/readc and print() calls not done with syscall
