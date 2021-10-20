from tkinter import *
import numpy as np
import  time

r = Tk()                                                                    #Master Window
r.geometry("420x405")
r.title("Sudoku Solver")
cur_instruction = StringVar()                                               #Instruction Variable 
cur_instruction.set("Enter Sudoku Board Below")
instruction_label = Label(r, font = "Consolas 14", textvariable=cur_instruction).pack()
t = Text(r, height=15, width=40, font="Consolas 14 bold")                   #Text Box
t.pack()

def solve():
    global substitution_counter
    substitution_counter = -1                                               #Remove one for the first call of solve()
    
    #Parsing Text from GUI input box
    while True:
        try:
            input_text = t.get("1.0",'end-1c')
            input_text = "".join(input_text.split())
            
            grid = np.zeros((9,9), dtype='int')
            for index, char in enumerate(input_text):
                grid[int(index/9)][index%9] = int(char)
            break
            
        except:
            cur_instruction.set( "Data format incorrect..")
            return

    cur_instruction.set( "Processing...")
    #Getting the set of numbers within a number's group------------------------------------
    dx = np.array([0,1,2,0,1,2,0,1,2])
    dy = np.array([0,0,0,1,1,1,2,2,2])
    def getgroupset(grid,groupx,groupy):
        s = set()
        tlx = groupx * 3                                                    #The x coordinate of the top left square in group
        tly = groupy * 3                                                    #The y coordinate of the top left square in group
        for i in range(9):
            s.add(grid[tly+dy[i]][tlx+dx[i]])
        return s

    #Function to check if a sudoku grid is solved------------------------------------------
    def check(grid):
        s = set()
        #Check rows
        for row in grid:
            s.clear()
            for value in row:
                s.add(value)
            if list(s) != [1,2,3,4,5,6,7,8,9]:
                return False
        #Check columns
        grid = grid.transpose()
        for column in grid:
            s.clear()
            for value in column:
                s.add(value)
            if list(s) != [1,2,3,4,5,6,7,8,9]:
                return False
        grid = grid.transpose()

        #Check groups (3x3 squares)
        for groupx in range(3):
            for groupy in range(3):
                s = getgroupset(grid, groupx, groupy)
                if list(s) != [1,2,3,4,5,6,7,8,9]:
                    return False
        return True

    #Recursive function to solve the grid--------------------------------------------------
    def solve(grid):
        if check(grid):                                                     #Return if solved (base case)
            return True, grid
        global substitution_counter
        substitution_counter +=1
        min_no = 10                                                         #The min number of possibilities
        fullset = set([1,2,3,4,5,6,7,8,9])                                  #Set of numbers in each row, column and group
        tgrid = grid.transpose()                                            #Transposed Grid
        for y, row in enumerate(grid):
            for x, value in enumerate(row):
                if value == 0:
                    setx = set(row)
                    sety = set(tgrid[x])
                    setgroup = getgroupset(grid, int(x/3), int(y/3))
                    possible = fullset - setx -sety - setgroup              #Set of possible values
                    
                    if len(possible) < min_no:
                        miny = y
                        minx = x
                        min_pos = possible
                        min_no = len(possible)
                        
        #print(min_pos)
        #print(minx, miny)                                             
        for possibility in min_pos:
            grid[miny][minx] = possibility
            solved, solved_grid = solve(np.copy(grid))                      #Important!! Ensure a copy of the array is passed into the recursive function
            if solved:
                return True, solved_grid
        return False, False


    #Main---------------------------------------------------------------------------------
    start = time.time()
    solved, grid = solve(grid)

    #Displaying output
    t.delete('1.0', 'end')
    if solved:
        cur_instruction.set( "Solved")
        t.insert('end', "Solved!!\n")
        t.insert('end', "Time Taken:" +  str(time.time()-start) + 's\n')
        t.insert('end', "Number of substitutions: " + str(substitution_counter) + '\n')
        t.insert('end', grid)
        print("Solved!")
        print("Time Taken:", str(time.time()-start) + 's')
        print("Number of substitutions", substitution_counter)
        print (grid)
    else:
        cur_instruction.set( "No Solution")
        print("Seems actually impossible...")
        t.insert('end', "Seems actually impossible...")
        
b = Button(r, text='solve!', command=solve, width =  '40', font ='Consolas 14 bold')
b.pack()
r.mainloop()




