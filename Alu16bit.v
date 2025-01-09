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
    