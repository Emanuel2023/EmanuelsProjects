`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////

module testbench();
    reg [15:0] A, B;
    reg Cin;
    reg [2:0] Control;
    wire [15:0] Result;
    wire Cout;
    
    ALU_16bit uut(A, B, Cin, Control, Result, Cout);
    
    initial begin
        $monitor("A=%b, B=%b, Cin=%b, Control=%b, Result=%b, Cout=%b", A, B, Cin, Control, Result, Cout);
        
        //A = 3'b010;
        //B = 3'b001;
        A = 16'b0000000000000010; // A = 2 in decimal
        B = 16'b0000000000000001;
        Cin = 1'b1;
        
        Control = 3'b000;
        #100;
        Control = 3'b001;
        #100;
        Control = 3'b010;
        #100;
        Control = 3'b011;
        #100;
        Control = 3'b100;
        #100;
        Control = 3'b101;
        #100;
        Control = 3'b110;
        #100;
        Control = 3'b111;
        #100;
     end
endmodule
        


