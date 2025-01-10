`timescale 1ns / 1ps

module IEEE_Multiplier(
    input [31:0] A,
    input [31:0] B,
    output reg [31:0] Result
);

    wire [7:0] Exponentof_A, Exponentof_B;
    wire [22:0] Mantissa_A, Mantissa_B;
    reg [47:0] Mantissa_Result;

    assign Exponentof_A = A[30:23];
    assign Exponentof_B = B[30:23];
    assign Mantissa_A = A[22:0];
    assign Mantissa_B = B[22:0];

    always @* begin
        Result[31] = A[31] ^ B[31];
        if ((Exponentof_A == 8'b0 && Mantissa_A == 23'd0) || (Exponentof_B == 8'b0 && Mantissa_B == 23'd0)) begin
            Result[30:23] = 8'b0;
            Result[22:0] = 23'd0;
        end
        else if ((Exponentof_A == 8'b1111_1111 && Mantissa_A == 23'd0) || (Exponentof_B == 8'b1111_1111 && Mantissa_B == 23'd0)) begin
            Result[30:23] = 8'b1111_1111;
            Result[22:0] = 23'd0;
        end
        else if ((Exponentof_A == 8'b1111_1111 && Mantissa_A != 23'd0) || (Exponentof_B == 8'b1111_1111 && Mantissa_B != 23'd0)) begin
            Result[30:23] = 8'b1111_1111;
            Result[22:0] = 23'd1;
        end
        else begin
            Mantissa_Result = {1'b1, Mantissa_A} * {1'b1, Mantissa_B};
            if (Mantissa_Result[47]) begin
                Result[30:23] = (Exponentof_A + Exponentof_B) - 127 + 1'b1;
                Result[22:0] = Mantissa_Result[46:24];
            end
            else begin
                Result[30:23] = (Exponentof_A + Exponentof_B) - 127;
                Result[22:0] = Mantissa_Result[45:23];
            end
        end
    end
endmodule


    
module testbench();

    reg [31:0] A, B;
    wire[31:0] Result;

IEEE_Multiplier uut(A, B, Result);

initial begin
    $monitor("A=%h, B=%h, Result=%h", A, B, Result);


    A = 32'h3fa00000;
    B = 32'h40200000;
    #100;
    A = 32'h4048f5c3;
    B = 32'h4048f5c3;
    #100;
    A = 32'h41c00000;
    B = 32'h42000000;
    #100;
    A = 32'h40b9999a;
    B = 32'h40866666;
   
    end
endmodule
