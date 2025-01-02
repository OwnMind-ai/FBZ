# FBZ: The Ultimate FizzBuzz Solution
Welcome to FBZ, the most efficient and elegant solution to the classic FizzBuzz problem. This project is designed to provide a highly optimized and easy-to-understand implementation, making it the go-to choice for developers seeking performance and clarity.

## Examples:
FizzBuzz:
```
/*
main - function name
(i) - iterator variable. If(i, f, s, ...), then 'f, s, ...' would be parameters
[0, 100] - range for iterator
N >> ... - print ... when i % N == 0
*/

main(i) [0, 100]:
  3 >> "FIZZ"
  5 >> "BUZZ"
  else >> i
end
```
Fibonacci:
```
main(i) [1, 1]:
  // The last arguments is the length of the sequence
  1 >> fibonacci(1, 0, 0, 20)
end

fibonacci(i, a, b, sum, n) [1, n]:
  (sum = a + b) + n >> "Shouldn't be printed"

  // Uncomment to print the whole sequence
  //1 >> sum

  (a = b) + (b = sum) + n >> "Shouldn't be printed"

  n >> "\nRESULT: "
  n >> sum
end
```
