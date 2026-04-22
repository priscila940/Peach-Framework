# Peach-Framework
Simple C++ Rendering Framework for DX11-DX12 called **Peach**. <br />
It was created as a **alternative for ImGui** that heavily relies on **CRT**. <br />
This is a **outdated example** for the most recent version that includes **Proper Glyph Rasterizing and STB** click <a href="https://github.com/MicrosoftARMAssembler/Divinity-Fortnite-Internal/tree/dfcdeccbfbe524a4cfac7cfac587a71dbd8273a3/workspace/core/render/peach">Here</a> <br />

**Why not ImGui?** <br />
ImGui relies on **CRT** dependencies like `malloc`, `memcpy`, and `sprintf` and **C++ operators** like `new` and `free`. <br />
For **manual mapped DLLs** it creates problems because **C++ operators** and **CRT functions** can't be resolved during mapping. <br />
ImGui also creates **font atlas textures** and leaves **byte patterns** in memory, combined with the **CRT** makes it a deadly detection for **Anti-Cheats**. <br />

This framework **requires** you to either **hijack or create your own Swapchain**. <br />
Here is the **interface** I created using Peach: <br />
<img width="1751" height="1274" alt="image" src="https://github.com/user-attachments/assets/3b738449-607f-4bfd-9b89-a5f8b54e1ea2" />
