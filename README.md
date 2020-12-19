# OSU_Portfolio
Portfolio Projects from OSU Courses

1) CS261 Data Structures - Hash Map ADT & Spell Checker [C]
    - Description : Chaining Hash Map ADT
    - Project Type: C99 Linux Makefile
    
    - Challenges 
        - hashMap.c 
            - Put & Remove Fxs - Looping array & traversing linked list 
            - hashMapRemove() - Filling void after removing a link

        - spellCecker.c
            - calcDistanace() - Implementing Levenshtein Distance Formula

    - Takeaways 
        - My main takeaway from implementing the Chained Hash Table was that you had to be
          deligent with links of the linked list and the member variables of the object. If
          you missed or did not time the assignment/method/operation involving either of these
          the Chained Hash Table would break. 

        - My main takeaway from the spellchecker was the process I went through of researching
          the calculation (which turned into a design of sorts) to implementing the function
          and trying to refine it. The research was key to making the implementation go smoothly.
          My extra time spent in research paid dividends in my implementation.

    
2) CS271 Computer Architecture and Assembly - Combination Calculator [MASM]
    - Description  : Combination Calculator
    - Project Type : x86 Assembly (MASM) in Visual Studio
    - Include Files: http://www.asmirvine.com/gettingStartedVS2019/index.htm#tutorial32
    - Challenge
        - Recursive calls - Managing data & stack
        - Procedures - Managing registers

    - Takeaways
        - The most important takeaway was that you need to be purposeful in the registers that
          you choose to hold data. You need to be thinking ahead and understand what operations
          need to be performed and what registers they may affect to make appropriate decisions.
          With that, if it is data that is not needed in the next set of calculations, then you
          have more freedom in choosing the register but you need to think of what data you need
          returning from procedures so the program can execute.


3) CS344 Operating Systems - smallsh [C]
    - Description : Simple shell with built-in functions
    - Project Type: C99 Linux Makefile
    - Challenges
        - Strings - Parsing, analyzing, and executing command line input
        - Redirection - Redirecting input/output for background and foreground execution
        - Child Processes - Tracking/Managing child processes
        - Signals - Changing signal behavior

    - Takeaways
        - I had many takeaways from this project. It was one that really challenged me as there were
          many details that needed to be addressed. That being said, the most important take aways for
          me were: parsing strings, redirecting input/output, managing child processes, and changing
          the behavior of signals.
