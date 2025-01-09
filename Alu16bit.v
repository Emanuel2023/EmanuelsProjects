`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////

module ALU_16bit(A, B, Cin, Control, Result, Cout);
    input [15:0] A, B;
    input Cin;
    input [2:0] Control;
    output reg [15:0] Result;
    output reg Cout;
    
    always @(*) begin
        case(Control)
          3'b000: {Cout, Result} <= A + B + Cin;
          3'b001: {Cout, Result} <= A - B - Cin;
          3'b010: {Cout, Result} <= A | B;
          3'b011: {Cout, Result} <= A & B;
          3'b100: {Cout, Result} <= {1'b0, A[14:0], 1'b0};//shift left(append 0)
          3'b101: {Cout, Result} <= {2'b00, A[15:1]};//shift right(append 0)
          3'b110: {Cout, Result} <= {1'b0, A[14:0], A[15]};
          3'b111: {Cout, Result} <= {1'b0, A[0], A[15:1]};
           endcase
         end
         
      endmodule        
    
    
    
    
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