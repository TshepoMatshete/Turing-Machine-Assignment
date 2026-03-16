# Turing-Machine-Assignment
This my solution for an assignment where I had to make a language encode and a Turing machine simulator

To compile the code for the **encoder** just simply run the command below:

g++ encoder.cpp -o encoder

then to run the script:

./encoder

you can also use cat to test run the code if you have .txt file that have sample inputs in the following way:

cat sampleTest.txt | ./encoder


To compile the code for the **Turing machine simplifier** that takes in a turing machine in the form: 
⟨𝐼𝑞𝑖,𝑠𝑛⟩=⟨𝑞𝑖⟩#⟨𝑠𝑛⟩#⟨𝑞𝑗⟩#⟨𝑠𝑚⟩#⟨𝐷⟩ where,

**Q** = { q₀, q₁, q₂, …, qₖ }  
**Γ** = { 0, 1 }  
**Δ** = { 0, 1, ⊔ }  
**q_init** = q₀, **q_accept** = q₁, **q_reject** = q₂  
**δ** = { I_{qᵢ,sₙ} | ∀ qᵢ ∈ Q / { q₁, q₂ }, ∀ sₙ ∈ Δ }

Given an instruction in the form

\( I_{q_i,s_n} = (q_i, s_n) \rightarrow (q_j, s_m, D) \),  
where \( D \in \{ S, L, L^2, R, R^2 \} \).

Given an instruction in the form:

\[
I_{q_i, s_n} = (q_i, s_n) \rightarrow (q_j, s_m, D)
\]

where:

\[
D \in \{ S, L, L^2, R, R^2 \}
\]

**Legend:**

- \(q_i\) — current state  
- \(s_n\) — symbol currently under the tape head  
- \(q_j\) — next state  
- \(s_m\) — symbol written to the tape  
- \(D\) — head movement:  
  - \(S\) = stay  
  - \(L\) = move left  
  - \(L^2\) = move left twice  
  - \(R\) = move right  
  - \(R^2\) = move right twice


Added a python script that runs tests for the turing machine simplifier **run_tests.py**

Usage:
    python3 run_tests.py <source.cpp> <tests_dir> [--timeout SECONDS] [--ignore-ws]

Test directory format:
    For each test case provide an input file and an expected output file.
    The script will pair files by basename. Allowed input extensions: .in, .input, .txt
    Allowed expected-output extensions: .out, .ans, .expected

Example:
    tests/
      case1.in
      case1.out
      case2.in
      case2.out


To run the c++ tester **test_runner** you follow the instructions below:

to compile:

g++ -std=c++17 test_runner.cpp -o test_runner

if you change the name of your simplifier you can go to line 27-29 and adjust the parameters of your **test_runner.cpp** or the directory of your file containing the tests is different. This **test_runner** will run without taking in inputs from the terminap