import math

# Define preprocessor variables
DEFAULT_BASE = 0
DEFAULT_HEIGHT = 0

class IsoTriangle:
    def __init__(self, base, height):
        self.set_base(base)
        self.set_height(height)

    def set_base(self, base):
        if base > 0:
            self.base = base
            self.calc_side()
            self.calc_perimeter()
            self.calc_area()
            self.calc_alpha()
            self.calc_beta()
        else:
            raise ValueError("Base must be greater than 0")

    def set_height(self, height):
        if height > 0:
            self.height = height
            self.calc_side()
            self.calc_perimeter()
            self.calc_area()
            self.calc_alpha()
            self.calc_beta()

    def calc_side(self):
        self.side = math.sqrt((self.base / 2.0) ** 2 + self.height ** 2)
        return self.side

    def calc_perimeter(self):
        self.perimeter = self.base + 2 * self.side
        return self.perimeter

    def calc_area(self):
        self.area = (self.base * self.height) / 2.0
        return self.area

    def calc_alpha(self):
        self.alpha = math.degrees(math.atan(self.height / (self.base / 2.0)))
        return self.alpha

    def calc_beta(self):
        self.beta = 180 - 2 * self.alpha
        return self.beta

    def print_all(self):
        print("------------------------------")
        print(f"Base      : {self.base}")
        print(f"Height    : {self.height}")
        print(f"Side      : {self.side}")
        print(f"Perimeter : {self.perimeter}")
        print(f"Area      : {self.area}")
        print(f"Alpha     : {self.alpha}")
        print(f"Beta      : {self.beta}")
        print("------------------------------")

# Testing the IsoTriangle class
i = IsoTriangle(6, 4)
i.print_all()