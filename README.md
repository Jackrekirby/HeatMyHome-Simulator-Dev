# Heating Simulator

This website simulates a range of domestic heating systems alongside solar photovoltaic and solar thermal technologies to optimise the system configuration and estimate the operation costs, capital costs and operating emissions for each of the technologies simulated. It takes as input the user's location and home details to find the optimal heating system unique to the user's circumstances.

The original simulator was written in Python, but has been rewritten in C++ for performance improvements and to allow the website to utilise Web Assembly. A Rust version has also been developed and compiled to WASM but was dropped due to poor performance on mobile iOS browsers.

Find the website here: https://jackrekirby.github.io/wasm_website/
