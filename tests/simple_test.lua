-- Simple test file for LuaJIT decompiler
local function factorial(n)
    if n <= 1 then
        return 1
    end
    return n * factorial(n - 1)
end

local function fibonacci(n)
    if n <= 1 then
        return n
    end
    return fibonacci(n - 1) + fibonacci(n - 2)
end

local function sum_array(arr)
    local sum = 0
    for i = 1, #arr do
        sum = sum + arr[i]
    end
    return sum
end

local numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
print("Sum of array: " .. sum_array(numbers))
print("Factorial of 5: " .. factorial(5))
print("Fibonacci of 10: " .. fibonacci(10))

-- Table operations
local data = {
    name = "test",
    value = 42,
    items = {"a", "b", "c"}
}

for k, v in pairs(data) do
    print(k .. " = " .. tostring(v))
end

-- Nested function
local function outer(x)
    local function inner(y)
        return y * 2
    end
    return inner(x) + x
end

print("Outer function result: " .. outer(5))

return {
    factorial = factorial,
    fibonacci = fibonacci,
    sum_array = sum_array,
    data = data,
    outer = outer
}