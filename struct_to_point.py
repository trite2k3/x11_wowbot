import re

# Define the Point class
class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __repr__(self):
        return f"Point({self.x}, {self.y})"

# The input C++ code
cpp_code = """
std::vector<Point> path = {
    {38.4314, 36.4706},
    {38.4314, 37.2549},
    {38.4314, 37.6471},
    {38.4314, 38.8235},
    {38.8235, 39.2157},
    {39.2157, 39.2157},
    {38.8235, 38.0392},
    {39.6078, 39.2157},
    {39.2157, 38.0392},
    {40, 38.4314},
    {40, 38.0392},
    {40, 37.6471},
    {40.3922, 36.8627},
    {39.6078, 37.6471},
    {39.2157, 37.2549},
    {38.8235, 36.0784},
    {38.8235, 36.4706},
};
"""

# Use regular expressions to find all the points
pattern = r'\s*\{\s*([\d.]+)\s*,\s*([\d.]+)\s*\}'
matches = re.findall(pattern, cpp_code)

# Convert matches to Point objects
path = [Point(float(x), float(y)) for x, y in matches]

# Print the result
print("path = [")
for point in path:
    print(f"    {point},")
print("]")
