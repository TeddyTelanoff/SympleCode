.global main
myVar$ = 4 # Set stack value of myVar to 0
out1$ = 8 # Set stack value of out1 to 4
myOther$ = 12 # Set stack value of myOther to 8
out2$ = 16 # Set stack value of out2 to 12
myStr$ = 24 # Set stack value of myStr to 16
.global .myStr
main: # Declare Function
	# Push Stack
	pushq %rbp
	mov %rsp, %rbp
	subq $4, %rsp # Allocate 4 bytes to the stack
	movl $69, -myVar$(%rbp) # Set myVar to $69
	subq $4, %rsp # Allocate 4 bytes to the stack
	movl -myVar$(%rbp), %eax
	addl $6, %eax
	movl %eax, -out1$(%rbp) # Move operation into out1
	subq $4, %rsp # Allocate 4 bytes to the stack
	# Set myOther to -myVar$(%rbp)
	movl -myVar$(%rbp), %eax
	movl %eax, -myOther$(%rbp)
	subq $4, %rsp # Allocate 4 bytes to the stack
	movl -myOther$(%rbp), %eax
	addl $5, %eax
	movl %eax, -out2$(%rbp) # Move operation into out2
	subq $8, %rsp # Allocate 8 bytes to the stack
	movabsq $.myStr, %rcx
	movq %rcx, -myStr$(%rbp)
	movl $0, %eax # Return $0
	popq %rbp # Pop Stack
	ret # Exit Function
.myStr:
	.asciz "lol gamer moment"
