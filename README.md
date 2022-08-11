# STM32L4-Bootloader (Bare-metal, Board Support Package, Register Level)

One of the important issues for embedded systems is that the software can be updated remotely. We are developing products that contain embedded software, and sometimes we need to update the embedded software in these products by making software additions and improvements based on errors in the codes or customer requests. In such cases, it would not be the right choice to update the new software we have developed by going to the beginning of the products. Moreover, thousands of these products may have been sold or even sent not only domestically but also abroad. It is an important feature that the product we have developed for such situations can be updated remotely.

What is wanted to be done in this project is to be able to change the user applications received by updating remotely thanks to the bootloader. By default, the Bootloader will offer the ability to update 2 applications that it stores in a different location on its flash while doing something desired. Flash is divided into 3 parts and is located at the base address (0x08000000) with the bootloader default mode turned on, APP-1 at 0x08040000 and APP-2 at 0x08080000. In this way, the UART connection is established and the files sent from the terminal are written instead of APP-1 or APP-2 and the code update is performed remotely. 

## Working Principle and User Manual

### Default Screen
![image](https://user-images.githubusercontent.com/70964563/184142733-299da9a9-5ba1-43bc-af73-1250fce1ec7a.png)

### If u are not click any key u can see this message 
![image](https://user-images.githubusercontent.com/70964563/184143040-c20154c8-c975-4687-bd5e-85eb32eb3319.png)

### If u are click any key u can see this menu
![image](https://user-images.githubusercontent.com/70964563/184143150-49a39bf6-e367-4506-a98f-e2b28a753949.png)

### If u are select 0 u can see this message and blink the pc7 and pb14 leds
![image](https://user-images.githubusercontent.com/70964563/184143444-bee2610b-d94e-4e5f-a8d8-79a3b74e1d18.png)

### If u are select 1 u can see this message and blink the pc7 led
![image](https://user-images.githubusercontent.com/70964563/184144426-eb21772c-69a1-47a2-9f20-a4960ee7de2a.png)

### If u are select 2 u can see this message and blink pb14 led
![image](https://user-images.githubusercontent.com/70964563/184144623-4c8e1dc4-4392-4d07-a897-4f0148ed56f9.png)

### If u are select 3 u can see this message and delete APP-1
![image](https://user-images.githubusercontent.com/70964563/184144785-4ef76eab-2337-40c6-99bd-4462125450db.png)

### if u are select 4 u can see this message and delete APP-2
![image](https://user-images.githubusercontent.com/70964563/184144955-66f484f9-416e-479e-be55-d900a8c8e93f.png)

### If u are select 5 u can see this sub menu and u must choose anything
![image](https://user-images.githubusercontent.com/70964563/184145131-8217bbe9-ee52-4818-8fab-6fbdc98ed050.png)

### If u are select 6 u can see this message and u can transfer .bin file instead of APP-1 from uart
![image](https://user-images.githubusercontent.com/70964563/184145542-a34f5b57-f12c-4869-b01e-706329fcc7e3.png)

### If u are select 7 u can see this message and u can transfer .bin file instead of APP-2  from uart
![image](https://user-images.githubusercontent.com/70964563/184145846-b87c6b97-35f7-4f6b-979b-9f9c9c03d7a5.png)





