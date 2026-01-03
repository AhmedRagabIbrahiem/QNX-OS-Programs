# QNX-OS-Programs
These are some QNX programs related to handling threads and processes
# CPU_Affinity
determines which cores your threads run on—an often overlooked performance lever that can make or break your application.

These traces show 4 threads running on a hashtag#QNX target, where each pair shares data.

The first test pins sharing threads to the same CPU—resulting in efficient cache utilization. The second scatters them across different cores, triggering costly cache coherency overhead.
The result: proper affinity saves 0.477ms—significant for latency-sensitive systems.

Beyond performance, affinity serves critical needs: isolating safety-critical ASIL applications from QM functions, ensuring real-time determinism, and managing power consumption. Each use case requires thoughtful core assignment.
Affinity isn't just scheduling—it's orchestrating your code to dance with your hardware's memory hierarchy. Get it right, and you turn cache into a performance superpower.


<img width="1045" height="736" alt="image" src="https://github.com/user-attachments/assets/ce754152-ab61-41f7-8092-c1d130108b74" />
