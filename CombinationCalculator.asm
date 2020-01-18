TITLE Combination Calculator     (comboCalculator.asm)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Author: Will Geller
; Last Modified: 12/23/2019
; OSU email address: gellerw@oregonstate.edu
; 
; Description: The program randomly generates combination problems (nCr) for a user to solve. 
;					1) Displays an introduction with a brief overview of the program
;					2) Randomly generates and displays a combination problem
;					3) User is prompted to enter their answer to the problem, s/b unsigned int
;					4) The input validated and the user is reprompted until entry is valid
;					5) The program calculates the solution of the combination problem
;					6) The answer is displayed, the user's answer is compared to the solution,
;					   and the program infroms the user if their answer was right or wrong.
;					7) The user is asked if they would like to solve another problem
;					8) The program loops to generate another problem if the user wants to solve
;					   another problem (loop to step 2).
;					9) When the user is finished the number of correct and incorect answers
;					   is displayed with a goodbye message.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

INCLUDE Irvine32.inc

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  GLOBAL CONSTANTS  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

MIN_N			EQU 3						; Minimum value of items in set (min of n)
MAX_N			EQU 12						; Maximum value of items in set (max of n)
MIN_R			EQU 1						; Minimum value of items selected (min of r)


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  MACROS  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Display a string and, optionally, display an unsigned integer after the string
output			MACRO outputStr:REQ, outputNum	
	push		edx

	mov			edx, OFFSET outputStr
	call		WriteString					;; Display string from offset of outputStr
	
	;; If second parameter is not blank
	IFNB		<outputNum>
		push	eax
		mov		eax, outputNum
		call	WriteDec					;; Display unsigned integer value
		pop		eax
	ENDIF

	pop			edx
ENDM

; Caclulate a random integer in the range [min, max] and store the value
randomNum		MACRO min:REQ, max:REQ, random:REQ
	push		eax

	mov			eax, max
	sub			eax, min
	inc			eax							;; max - min + 1 is the desired size of the output range
	call		RandomRange					;; Return random integer [0 ... (max - min)]
	add			eax, min					;; Adjust random integer to be in range [min, max]
	mov			random, eax					;; Store random integer to memory or register (cannot be EAX)
	
	pop			eax
ENDM


.data
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  STRING VARIABLES  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; String variables for introduction procedure
progTitle		BYTE "Welcome to the Combinations Calculator!",0Dh,0Ah,0
author			BYTE "Implemented by Will Geller",0Dh,0Ah,0
inform1			BYTE "I'll give you a random combination problem to solve.",0Dh,0Ah,0
inform2			BYTE "Enter your answer and I'll let you know if it's correct.",0Dh,0Ah,0

; String variables for showProblem procedure
probTitle		BYTE "Combination Problem #",0
setItems		BYTE "Unique items in set (n) = ",0
subItems		BYTE "Items selected from set (r) = ",0

; String variables for getData procedure
howMany			BYTE "How many combinations are there? (nCr): ",0
invalidEntry	BYTE "Your entry was invalid. Please enter digits (0 - 9) only.",0

; String variables for showResult procedure
solution1		BYTE "There are ",0
solution2		BYTE " combinations of ",0
solution3		BYTE " from a set of ",0
correct			BYTE "You kow what you are doing. You are correct!",0Dh,0Ah,0
incorrect		BYTE "Go back for some review. Your answer is incorrect...",0Dh,0Ah,0

; String variables for anotherProblem and goodbye procedures
newProblem		BYTE "Fancy another problem? (Y/N): ",0
invalidChar		BYTE "Invalid response. Please enter a 'Y' or an 'N'.",0
dashes			BYTE 80 DUP('-'),0Dh,0Ah,0
numCorrect		BYTE "Correct Answers  : ",0
numIncorrect	BYTE "Incorrect Answers: ",0
goodbyeMsg		BYTE 0Dh,0Ah,"Thanks for using the Combinations Calculator!",0Dh,0Ah,0


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  VARIABLES  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Variables for the combination problem
nSet			DWORD ?						; Unique items in set, n, and max value for r
rSub			DWORD ?						; Number of items selected from set (r)
strAnswer		BYTE 10 DUP(?)				; User's string answer
maxLength		DWORD ($ - strAnswer)		; Max input is 10 characters to avoid truncated entries
answer			DWORD ?						; User's validated answer as unsigned integer
result			DWORD ?						; Calculated solution, nCr = n! / (r! * (n - r)!)

; Variables for accumulating and loop control
score			DWORD 0						; Number of correct answers
problemNum		DWORD 0						; Problem number the user is on
continue		DWORD ?						; Character for user's continue response
loopCtrl		DWORD 'Y'					; Loop control variable, holds value to continue loop


.code
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PROCEDURE    : main
; DESCRIPTION  : Controls the program flow: pushes parameters, calls procedures, tracks
;				 the number of problems (number of loops), and controls the program loop.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

main PROC
	call		introduction				; Display the introduction string parameters

comboProblem:
	output		dashes						; Display a visual break on console with dashes
	call		CrLf
	inc			problemNum
	
	push		problemNum					; Parameter: Problem number by val
	push		OFFSET nSet					; Parameter: Number of unique items in set (n) by ref
	push		OFFSET rSub					; Parameter: Number of items selected from set (r) by ref
	call		showProblem					; Randomly generate & store values for n and r, display problem

	push		maxLength					; Parameter: Max length of input by val
	push		OFFSET strAnswer			; Parameter: User input, string of digits, by ref
	push		OFFSET answer				; Parameter: User's answer, validated unsigned integer, by ref
	call		getData						; Get user's answer, validate, and store decimal value in answer

	push		OFFSET result				; Parameter: Result of combination problem by ref
	push		nSet						; Parameter: Number of unique items in set (n) by val
	push		rSub						; Parameter: Number of items to be selected (r) by val
	call		combinations				; Calculate combination problem solution and store in result
	
	push		nSet						; Parameter: Number of unique items in set (n) by val
	push		rSub						; Parameter: Number of items to be selected (r) by val
	push		result						; Parameter: Result of combination problem by val
	push		answer						; Parameter: User's answer, unsigned integer, by val
	push		OFFSET score				; Parameter: Number of correct answers by ref
	call		showResults					; Display result and add to score if answer is correct

	push		OFFSET continue				; Parameter: Continue character response by ref
	call		anotherProblem				; Get user's response to continue and solve another problem

; Determine if user wants to solve anoteher problem	
	mov			eax, continue
	cmp			eax, loopCtrl				; Compare user's response to the loop control				
	je			comboProblem				; If the user wants to continue, loop/generate new problem
	
	output		dashes						; Display dashes as a visual break on console
	call		CrLf

	push		score						; Parameter: Number of correct answers by val
	push		problemNum					; Parameter: Number of problems attempted by val
	call		goodbye						; Display number of correct/incorrect answers & goodbye message
	
	exit
main ENDP


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PROCEDURE    : introduction
; DESCRIPTION  : Displays title and author of program followed by two informative messages
;				 describing what the program does.
; RECEIVES     : n/a
; RETURNS	   : n/a
;
; PRECONDITIONS: 1) The output macro is implemented
;				 2) String variables progTitle, author, inform1, & inform2 are initialized
; CHANGES	   : n/a
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

introduction PROC
	enter		0, 0
	
	output		progTitle					; Display program title
	output		author						; Display program author
	call		CrLf
	output		inform1						; Infroms user the program generates a problem
	output		inform2						; Informs user they enter answer and results are shown
	call		CrLf
	
	leave
	ret
introduction ENDP

 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PROCEDURE    : showProblem
; DESCRIPTION  : Generates a combination problem, nCr, by randomly generating unsigned
;				 integers for n and r. The value of n is in the range [3, 12] and r is in
;				 the range [1, n]. Both values are stored and the problem is displayed.
; RECEIVES     : 1) Problem number, probNum by val
;				 2) Offset for value of n, nSet by ref
;				 3) Offset for value of r, rSub by ref
; RETURNS	   : The values of n and r will be stored in nSet and rSub, respectively
;
; PRECONDITIONS: 1) Global constants MIN_N, MAX_N, & MIN_R are initialized
;				 2) Macros output & randomNum are implemented
;			     3) String variables probTitle, setItems, & subItems are initialized
;				 4) probNum is initialized and has a value >= 1
;				 4) Variables nSet & rSub are declared but uninitialized
; CHANGES	   : n/a
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

showProblem PROC
	enter		0, 0
	pushad

	call		Randomize					; Initialize seed for random numbers
	mov			ebx, [ebp + 16]				; EBX - Value of problem number
	mov			edi, [ebp + 12]				; EDI - Offset of nSet
	mov			esi, [ebp + 8]				; ESI - Offset of rSub
	
; Generate random combination problem
	randomNum	MIN_N, MAX_N, [edi]			; Generate random value for n [MIN_N, MAX_N]
	mov			edx, [edi]
	randomNum	MIN_R, edx, [esi]			; Generate random value for r [MIN_R, n]	
	
	
; Display the problem
	output		probTitle, ebx				; Displays problem number
	call		CrLf
	output		setItems, [edi]				; Displays string and value for n
	call		CrLf
	output		subItems, [esi]				; Displays string and value for r
	call		CrLf
		
	popad
	leave
	ret			12
showProblem ENDP


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PROCEDURE    : getData
; DESCRIPTION  : Gets the user's answer to the problem as a string and store in userInput.
;				 Validates that the string contains only digits comparing characters to
;				 the ASCII values for 0 and 9. Displays an error message and reprompts the
;				 user if a character is not a digit. Valid entries are converted to
;				 decimal and stored in answer.
; RECEIVES     : 1) Max input length, inputLen by val
;				 2) Offset for user's answer as a string, strAnswer by ref
;				 3) Offset for user's validated decimal answer, answer by ref
; RETURNS	   : User's answer is stored in strAnswer & answer as a string of digits &
;				 decimal value, respectively
;
; PRECONDITIONS: 1) The outputMacro has been implemented
;				 2) String variables howMany & invalidEntry are initialized
;				 3) Variable inputLen is initialized and is a positive integer greater than
;					the number of digits in the largest posssible answer
;				 4) Variables strAnswer & answer are declared but unitialized
; CHANGES	   : n/a
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

getData PROC 
	LOCAL		char0:BYTE,					; Hold ASCII value of 0 digit
				char9:BYTE,					; Hold ASCII value of 9 digit
				decBase:DWORD				; Hold decimal base
	pushad

; Assign value to local variables
	mov			char0, 30h					; Assign local the ASCII value for 0
	mov			char9, 39h					; Assign local the ASCII value for 9
	mov			decBase, 10					; Assign local the decimal base value
	
getAnswer:
	mov			ecx, [ebp + 16]				; ECX - Max input length
	mov			edx, [ebp + 12]				; EDX - Offset of strAnswer
	output		howMany						; Prompt the user to enter their answer
	call		ReadString					; Read user input and store as string
	
	push		eax							; Store length of the user's entry
	mov			ecx, eax					; Set counter to length of user entry
	mov			esi, [ebp + 12]				; ESI - Offset of strAnswer
	cld										; Clear direction flag to get characters from beginning to end

validate:
	lodsb									; Load character and increment ESI
	cmp			al, char0					; Compare ASCII value for 0 to character value
	jl			notDigit					; If char value is less, value is not a digit and jump to error message
	cmp			al, char9					; Compare character value to ASCII for 9
	jg			notDigit					; If char value is greater, value is not a digit and jump to error message
	loop		validate					; Loops until each character has been validated as a digit
	jmp			validEntry					; The user entry is valid, jump to convert the digit string to decimal

notDigit:
	pop			eax							; Restore length of user input
	call		CrLf
	output		invalidEntry				; Display invalid entry message
	call		CrLf
	jmp			getAnswer					; Jump to reprompt for valid entry

validEntry:
	pop			eax							; Restore length of user input
	mov			ecx, eax					; Set counter to length of user entry
	dec			esi							; ESI - strAnswer last digit's offset
	mov			edi, [ebp + 8]				; EDI - Offset for answer
	mov			eax, 0						; Clear EAX
	mov			[edi], eax					; Set EDI to 0 to accumulate the decimal value entered by the user
	mov			ebx, 1						; EBX - Decimal position, starting at 1
	std										; Set direction flag to get digits from end to beginning

charToDec:
	lodsb									; Load digit and decrement ESI
	and			eax, 0Fh					; Bitwise AND to convert ASCII value to decimal
	mul			ebx							; Multiply decimal value by decimal position
	add			[edi], eax					; Add decimal value to answer
	mov			eax, ebx
	mul			decBase						; Multiply by decimal base to increment one decimial position
	mov			ebx, eax					; Store new decimal position
	loop		charToDec					; Loop until each digit is converted and decimal value of answer is stored
	call		CrLf

	popad
	ret			12
getData ENDP


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PROCEDURE    : combinations
; DESCRIPTION  : Calculates the number of combinations of nCr and stores the result. The
;				 formula is: nCr = (n! / (r! * (n - r)!)
; RECEIVES     : 1) Offset for calculated solution, result by ref
;				 2) Value of n, nSet by val
;				 3) Value of r, rSub by val
; RETURNS	   : The number of combinations r items can be selected from n stored in result
;
; PRECONDITIONS: 1) The factorial procedure has been implemented
;				 2) result is declared but uninitialized
;			     3) nSet and rSub have been assigned integer values such that 0 < r <= n
; CHANGES	   : n/a
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

combinations PROC
	enter		0, 0
	pushad
	
	mov			edi, [ebp + 16]				; EDI - Offset of result
	mov			eax, [ebp + 8]
	cmp			eax, [ebp + 12]				; Compare value of n and r
	je			nEqualsR					; Jump to end if they are equal

; Calculate r!, part of the divisor
	push		eax							; Parameter: Value of r
	push		edi							; Parameter: Offset of result
	call		factorial					; Calculate r!
	mov			ebx, [edi]					; Store r! in EBX
	xor			[edi], ebx					; Bitwise XOR to set result to 0


; Calculate (n - r)!, part of divisor
	mov			eax, [ebp + 12]
	sub			eax, [ebp + 8]				; Calculate (n - r)
	push		eax							; Parameter: Value of (n - r)
	push		edi							; Parameter: Offset of Result
	call		factorial					; Calculate (n - r)!

; Calculate r! * (n - r)!, the divisor
	mov			eax, [edi]					; Move (n - r)! to EAX for multiplication
	xor			[edi], eax					; Bitwise XOR to set result to 0
	mul			ebx							; Multiply r! * (n - r)! to calculate divisor
	mov			ebx, eax					; Store divisor in EBX

; Calculate n!, the dividend
	mov			eax, [ebp + 12]
	push		eax							; Parameter: Value of n 
	push		edi							; Parameter: Offset of Result
	call		factorial					; Calculate n!
	
; Calculate n! / (r! * (n - r)!) for solution
	mov			eax, [edi]					; Move n! to EAX for division
	mov			edx, 0						; Zero extend EDX for division
	div			ebx							; Divides n! by r! * (n - r)!
	mov			[edi], eax					; Store calculated solution in result
	jmp			calcComplete

nEqualsR:
	mov			eax, 1						
	mov			[edi], eax					; If r = n, the answer is 1

calcComplete:
	popad
	leave
	ret			12
combinations ENDP


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PROCEDURE    : factorial
; DESCRIPTION  : Calculates the factorial of a positive integer using recursion. The base
;		  	     case is when the integer is equal to 1.
; RECEIVES	   : 1) Positive integer by val
; 				 2) Offset for calculated solution, result by ref
; RETURNS	   : Factorial of positive integer is stored in result
;
; PRECONDITIONS: 1) The integer is a positive integer (>= 1)
;				 2) The value at result is 0 when function is called
; CHANGES	   : n/a
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

factorial PROC
	enter		0, 0
	pushad
	
; Base case check
	mov			eax, [ebp + 12]
	cmp			eax, 1						; Compare positive integer to 1, base case
	je			baseReturn					; Jump to baseReturn if parameter equals 1

	mov			edi, [ebp + 8]				; EDI - Offset of result
	dec			eax
	push		eax							; Parameter: Value of [positive integer] - 1
	push		edi							; Parameter: Offset of result
	call		factorial					; Recursive procedure call
	
factorialReturn:
	mov			eax, [edi]					; EAX - Recursive call return value
	mov			ebx, [ebp + 12]				; EBX - Positive integer
	mul			ebx							; Product is factorial of positive integer

; For base case, value stored is 1 and returned
baseReturn:
	mov			[edi], eax					; Store factorial in result

	popad
	leave
	ret			8
factorial ENDP


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PROCEDURE    : showResults
; DESCRIPTION  : Displays the solution, compares solution to the user's answer, and, based
;				 on the comparison, displays a result message (correct or incorrect). If
;				 the answer is correct, the score is incremented.
; RECEIVES     : 1) Value of n, nSet by val
;				 2) Values of r, rSub by val
;				 3) Value of calculated solution, result by val
;				 4) Value of user's answer, answer by val
;				 5) Offset of number of correct answers, score by ref
; RETURNS	   : Score is incremented if correct, otherwise n/a
; PRECONDITIONS: 1) The output macro has been implemented
;				 2) String variables solution1, solution2, solution3, correct, & incorrect
;				    are initialized
;				 3) All variables passed as parameters have an integer value assigned
; CHANGES	   : n/a
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

showResults PROC
	enter		0, 0
	push		eax
	push		edi


; Display the solution output (displays as one sentence)
	output		solution1, [ebp + 16]
	output		solution2, [ebp + 20]
	output		solution3, [ebp + 24]
	call		CrLf

; Determine if the user's answer is correct
	mov			edi, [ebp + 8]				; EDI - Number of correct answer's, score
	mov			eax, [ebp + 16]				; EAX - Calculated solution, result
	cmp			eax, [ebp + 12]				; Compare user's answer to the solution
	jne			wrongAnswer					; If they are not the same, jump to wrongAnswer
	output		correct						; Display correct answer message
	mov			eax, 1
	add			[edi], eax					; Add 1 to score
	jmp			complete					; Jump to return to caller

wrongAnswer:
	output		incorrect					; Display incorrect answer message

complete:
	call		CrLf
	pop			edi
	pop			eax
	leave
	ret			20
showResults ENDP


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PROCEDURE    : anotherProblem
; DESCRIPTION  : Asks the user if they want to solve another problem and reads their input.
;				 If the input is invalid, an error message is displayed and the user is
;			     reprompted. If the entry is valid, the value is stored.
; RECEIVES     : 1) Offset of user's response, continue by ref
; RETURNS	   : n/a
; PRECONDITIONS: 1) The output macro has been implemented
;				 2) String variables newProblem & invalidChar are initialized
;			     3) continue has been declared
; CHANGES	   : n/a
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

anotherProblem PROC
	LOCAL		charY:BYTE,
				charN:BYTE
	push		eax
	push		edi
	
	mov			charY, 59h					; Assign charY the ASCII value of 'Y'
	mov			charN, 4Eh					; Assign charN the ASCII value of 'N'
	mov			edi, [ebp + 8]				; EDI - Offset of continue
	
getCharacter:
	output		newProblem					; Ask the user if they want to solve another
	call		ReadChar					; Read character response from user
	and			eax, 0DFh					; Bitwise AND to change to uppercase and clear higher bytes
	call		WriteChar					; Display the user's input (uppercase letter if letter)
	call		CrLf
	mov			[edi], eax					; Store the user's response
	cmp			al, charY					; Compare entry to ASCII value of 'Y'
	je			validEntry					; Jump to return if valid
	cmp			al, charN					; Compare entry to ASCII value of 'N'
	je			validEntry					; Jump to return if valid
	output		invalidChar					; Display error message
	call		CrLf
	jmp			getCharacter				; Jump to reprompt until entry is valid

validEntry:
	call		CrLf
	pop			edi
	pop			eax
	ret			4
anotherProblem ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PROCEDURE    : goodbye
; DESCRIPTION  : Calculates the number of incorrect answers. Displays the number of
;				 correct answers and incorrect answers followed by a goodbye message.
; RECEIVES     : 1) Number of correct answers, score, by val
;				 2) Number of problems attempted, problemNum, by val
; RETURNS	   : n/a
; PRECONDITIONS: 1) Output macro is implemented
;				 2) String variables numCorrect1, numCorrect2, numCorrect3, & goodbyeMsg
;					are initialized
;				 3) score & problemNum hold integers such that 0 <= score <= problemNum
; CHANGES	   : n/a
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

goodbye PROC
	enter		0, 0
	push		ebx	

	mov			ebx, [ebp + 8]				
	sub			ebx, [ebp + 12]				; The difference equals incorrect answers
	output		numCorrect, [ebp + 12]		; Display correct answers
	call		CrLf
	output		numIncorrect, ebx			; Display incorrect answers
	call		CrLf
	output		goodbyeMsg					; Display goodbye message
	call		CrLf

	pop			ebx
	leave
	ret		8

goodbye ENDP


END main
