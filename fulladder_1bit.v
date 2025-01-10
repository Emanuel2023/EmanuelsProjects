`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 09/04/2024 10:05:20 AM
// Design Name: 
// Module Name: fulladder
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module Full_Adder (A, B, Cin, Sum, Cout);
    input A, B, Cin;    
    output Sum, Cout;


    // Sum is calculated as the XOR of A, B, and Cin
    assign Sum = (A ^ B) ^ Cin;

//    // Carry out is calculated using OR for proper logical operation
    assign Cout = ((A ^ B) & Cin) | (A & B);

endmodule


module tb();
    reg A, B, Cin;
    wire Sum, Cout;
    
    Full_Adder uut(A, B, Cin, Sum, Cout );
    
    initial begin 
        A = 0; B = 0; Cin = 0;
        
        $monitor("A=%b, B=%b, Cin=%b, Sum=%b, Cout=%b", A, B, Cin, Sum, Cout);
        A = 0; B = 0; Cin = 0;  // Expected: Sum = 0, Cout = 0
        #100;
        A = 0; B = 0; Cin = 1;  // Expected: Sum = 1, Cout = 0
        #100;
        A = 0; B = 1; Cin = 0;  // Expected: Sum = 1, Cout = 0
        #100;
        A = 0; B = 1; Cin = 1;  // Expected: Sum = 0, Cout = 1
        #100;
        A = 1; B = 0; Cin = 0;  // Expected: Sum = 1, Cout = 0
        #100;
        A = 1; B = 0; Cin = 1;  // Expected: Sum = 0, Cout = 1
        #100;
        A = 1; B = 1; Cin = 0;  // Expected: Sum = 0, Cout = 1
        #100;
     
        A = 1; B = 1; Cin = 1;  // Expected: Sum = 1, Cout = 1
        #100;
    end
        
endmodule