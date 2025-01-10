
/*
 * snake_top.v
 *
 * Henry Kroeger & Sarah Chow
 * EE 364 Final Project
 *
 * Top control for the snake game.
 */

module snake_top (ddr2_cas_n, ddr2_we_n, QuadSpiFlashCS,
                        ClkPort, BtnL, BtnR, BtnU, BtnD, BtnC, Sw0,
                        Ld7, Ld6, Ld5, Ld4, Ld3, Ld2, Ld1, Ld0,
                        Dp, Cg, Cf, Ce, Cd, Cc, Cb, Ca,
                        An7, An6, An5, An4, An3, An2, An1, An0,
                        hSync, vSync, vgaR, vgaG, vgaB);

// Inputs
input ClkPort;
input BtnL, BtnR, BtnU, BtnD, BtnC;
input Sw0;

// Outputs

output      ddr2_we_n, ddr2_cas_n, QuadSpiFlashCS;

output Ld7, Ld6, Ld5, Ld4, Ld3, Ld2, Ld1, Ld0;
output Dp, Cg, Cf, Ce, Cd, Cc, Cb, Ca;
output An7, An6, An5, An4, An3, An2, An1, An0;

output hSync, vSync;
output [3:0] vgaR, vgaG, vgaB;

// Clock Signals
wire ClkPort;

wire board_clk, game_clk, ssd_clk;
reg [32:0] div_clk;

// Other
wire reset;
wire Qi, Qm, Qc, Qh, Qe, Qw, Ql, Qu;
wire [7:0] food;
wire [3:0] length;

// cannot input/output byte arrays in Verilog (without SystemVerilog). instead, we pack the byte array into a 128-bit
// wire and i/o this wire
wire [127:0] locations_flat;

// DC, SC Outputs
wire [9:0] vc;
wire [9:0] hc;
wire [11:0] rgb;
wire [11:0] background;

assign vgaR = rgb[11 : 8];
assign vgaG = rgb[7  : 4];
assign vgaB = rgb[3  : 0];

// SSD
reg [7:0] ssd;
wire ssd1;
wire [3:0] ssd0;
reg [7:0] ssd_cathodes;


assign reset = Sw0;

// disable memory ports
assign { ddr2_we_n, ddr2_cas_n, QuadSpiFlashCS} = 4'b1111;

// Clock Division
BUFGP BUFGP1 (board_clk, ClkPort);
always @(posedge board_clk, posedge reset)
begin
      if (reset) begin
            div_clk <= 0;
      end
      else begin
            div_clk <= div_clk + 1'b1;
      end
end

assign game_clk = div_clk[22];
assign vga_clk = div_clk[19];



snake_core snake_core1 (.Left(BtnL), .Right(BtnR), .Up(BtnU), .Down(BtnD), .Ack(BtnC), .Reset(reset), .Clk(game_clk), .Qi(Qi), .Qm(Qm), .Qc(Qc), .Qh(Qh), .Qe(Qe),
                              .Qw(Qw), .Ql(Ql), .Qu(Qu), .Food(food), .Length(length), .Locations_Flat(locations_flat));

display_controller dc(.clk(board_clk), .hSync(hSync), .vSync(vSync), .bright(bright), .hCount(hc), .vCount(vc));
snake_controller sc(.Clk(vga_clk), .Bright(bright), .Reset(BtnC), .Qi(Qi), .Qw(Qw), .Ql(Ql), .Qc(Qc), .hCount(hc), .vCount(vc), .Food(food), .Length(length), .Locations_Flat(locations_flat), .rgb(rgb), .background(background));
      

assign {Ld7, Ld6, Ld5, Ld4} = {Qi, Qm, Qc, Qh};
assign {Ld3, Ld2, Ld1, Ld0} = {Qe, Qw, Ql, Qu};


assign ssd_clk = div_clk[18];

assign ssd1 = (length >= 10) ? 1'b1 : 1'b0;
assign ssd0 = (length % 10);

assign An7 = 1'b1;
assign An6 = 1'b1;
assign An5 = 1'b1;
assign An4 = 1'b1;
assign An3 = 1'b1;
assign An2 = 1'b1;
assign An1 = ~ssd_clk;
assign An0 = ssd_clk;

always @ (ssd_clk, ssd1, ssd0)
begin
      case (ssd_clk)
            1'b1: ssd = {4'b0000, ssd1};
            1'b0: ssd = {7'b0000000, ssd0};
      endcase
end

assign {Ca, Cb, Cc, Cd, Ce, Cf, Cg, Dp} = {ssd_cathodes};

always @( ssd )
begin
      case (ssd)
            8'd0: ssd_cathodes = 8'b00000011; // 0
            8'd1: ssd_cathodes = 8'b10011111; // 1
            8'd2: ssd_cathodes = 8'b00100101; // 2
            8'd3: ssd_cathodes = 8'b00001101; // 3
            8'd4: ssd_cathodes = 8'b10011001; // 4
            8'd5: ssd_cathodes = 8'b01001001; // 5
            8'd6: ssd_cathodes = 8'b01000001; // 6
            8'd7: ssd_cathodes = 8'b00011111; // 7
            8'd8: ssd_cathodes = 8'b00000001; // 8
            8'd9: ssd_cathodes = 8'b00001001; // 9
            endcase
end


endmodule