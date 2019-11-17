#!/bin/bash
IFS=$'\n'
#set -Eexuo pipefail
# Name: Hoikin Cheng

# Retriever program test script
# Retriever [targethost] [path] [port]
# For example: ./Retriever "www.google.com" "/" "80"

# Please run StartServer.sh in another console window before running this script


printf "Accessing a real server...\n"
printf "Console output:\n"
./Retriever https://www.google.com / 80
printf "\nDisplaying contents of the Output file:\n"
cat Output.txt
printf "\n\n\n"


printf "\nAccessing a file(TestFile.txt) on Server...\n"
printf "Console output:\n"
./Retriever $1 /TestFile.txt $2
printf "Displaying contents of the Output.txt file:\n"
cat Output.txt
printf "\n"
printf "Testing Output.txt against expected output...\n"
mapfile -t ARRAY2 < Output.txt
if [ "${ARRAY2[0]}" = "All work and no play makes Jack a dull boy," ] && 
   [ "${ARRAY2[1]}" = "All play and no work makes Jack a mere toy." ]; then
    printf "Test passed!\n\n\n"
else
    printf "Test failed!\n\n\n"
fi


printf "Accessing an unauthorized file (SecretFile.html)...\n"
printf "Console output:\n"
./Retriever $1 /SecretFile.html $2
printf "Testing console output against expected output...\n"
mapfile -t ARRAY3 < <(./Retriever $1 /SecretFile.html $2)
if [ "${ARRAY3[0]}" = "<html>" ] && [ "${ARRAY3[1]}" = "<head>" ] && 
   [ "${ARRAY3[2]}" = "<title>401 Unauthorized</title>" ] && 
   [ "${ARRAY3[3]}" = "</head>" ] && [ "${ARRAY3[4]}" = "<body>" ] && 
   [ "${ARRAY3[5]}" = "<h1>401 - Unauthorized</h1>" ] && 
   [ "${ARRAY3[6]}" = "</body>" ] && [ "${ARRAY3[7]}" = "</html>" ]; then
    printf "Test passed!\n\n\n"
else
    printf "Test failed!\n\n\n"
fi


printf "Accessing a forbidden file (../TestFile.txt)...\n"
printf "Console output:\n"
./Retriever $1 ../TestFile.txt $2
printf "Testing console output against expected output...\n"
mapfile -t ARRAY4 < <(./Retriever $1 ../TestFile.txt $2)
if [ "${ARRAY4[0]}" = "<html>" ] && [ "${ARRAY4[1]}" = "<head>" ] && 
   [ "${ARRAY4[2]}" = "<title>403 Forbidden</title>" ] && 
   [ "${ARRAY4[3]}" = "</head>" ] && [ "${ARRAY4[4]}" = "<body>" ] && 
   [ "${ARRAY4[5]}" = "<h1>403 - Forbidden</h1>" ] && 
   [ "${ARRAY4[6]}" = "</body>" ] && [ "${ARRAY4[7]}" = "</html>" ]; then
    printf "Test passed!\n\n\n"
else
    printf "Test failed!\n\n\n"
fi


printf "Accessing a file that does not exist...\n"
printf "Console output:\n"
./Retriever $1 TestFile.txt $2
printf "Testing console output against expected output...\n"
mapfile -t ARRAY5 < <(./Retriever $1 TestFile.txt $2)
if [ "${ARRAY5[0]}" = "<html>" ] && [ "${ARRAY5[1]}" = "<head>" ] && 
   [ "${ARRAY5[2]}" = "<title>404 Not Found</title>" ] && 
   [ "${ARRAY5[3]}" = "</head>" ] && [ "${ARRAY5[4]}" = "<body>" ] && 
   [ "${ARRAY5[5]}" = "<h1>404 - Not Found</h1>" ] && 
   [ "${ARRAY5[6]}" = "</body>" ] && [ "${ARRAY5[7]}" = "</html>" ]; then
    printf "Test passed!\n\n\n"
else
    printf "Test failed!\n\n\n"
fi


printf "Accessing a file using a bad request...\n"
printf "Console output:\n"
./Retriever $1 /TestFile.txt $2 0
printf "Testing console output against expected output...\n"
mapfile -t ARRAY6 < <(./Retriever $1 /TestFile.txt $2 0)
if [ "${ARRAY6[0]}" = "<html>" ] && [ "${ARRAY6[1]}" = "<head>" ] && 
   [ "${ARRAY6[2]}" = "<title>400 Bad Request</title>" ] && 
   [ "${ARRAY6[3]}" = "</head>" ] && [ "${ARRAY6[4]}" = "<body>" ] && 
   [ "${ARRAY6[5]}" = "<h1>400 - Bad Request</h1>" ] && 
   [ "${ARRAY6[6]}" = "</body>" ] && [ "${ARRAY6[7]}" = "</html>" ]; then
    printf "Test passed!\n\n\n"
else
    printf "Test failed!\n\n\n"
fi
