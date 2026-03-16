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
