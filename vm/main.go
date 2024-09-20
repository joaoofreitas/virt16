package main

import (
	"fmt"
	"os"
	"time"

	tea "github.com/charmbracelet/bubbletea"
	"github.com/charmbracelet/bubbles/table"
	"github.com/charmbracelet/lipgloss"
)

type model struct {
	vm          *VM
	memoryTable table.Model
	status      string
	executing   bool

	windowWidth  int
	windowHeight int

	displayBuffer []byte // Store ASCII display characters
}

type tickMsg time.Time

var (
	// Colors and borders
	borderStyle = lipgloss.NewStyle().Border(lipgloss.NormalBorder()).Padding(0, 0).BorderForeground(lipgloss.Color("240"))
	emptyRow = lipgloss.NewStyle().PaddingLeft(1).Render
)

func NewModel() model {
	vm := NewVM()

	// Memory Table setup (columns for 0x0 to 0xF)
	columns := []table.Column{
		{Title: "Addr", Width: 7},
	}
	for i := 0; i < 16; i++ {
		columns = append(columns, table.Column{Title: fmt.Sprintf("0x%X", i), Width: 6})
	}

	// Memory is organized in rows of 16 values
	rows := make([]table.Row, 0)
	for addr := 0; addr < 32768; addr += 16 {
		row := make([]string, 17)
		row[0] = fmt.Sprintf("0x%04X", addr) // First column is the base address
		for col := 0; col < 16; col++ {
			row[col+1] = fmt.Sprintf("0x%02X", vm.Memory[addr+col])
		}
		rows = append(rows, row)
	}

	t := table.New(
		table.WithColumns(columns),
		table.WithRows(rows),
		table.WithFocused(true),
		table.WithHeight(20),  // Placeholder height, will dynamically change
		table.WithKeyMap(table.DefaultKeyMap()),
	)

	return model{
		vm:            vm,
		memoryTable:   t,
		status:        "Ready",
		executing:     false,
		displayBuffer: make([]byte, 64*64), // For 64x64 ASCII display
	}
}

func (m model) Init() tea.Cmd {
	return tea.EnterAltScreen
}

func (m model) Update(msg tea.Msg) (tea.Model, tea.Cmd) {
	switch msg := msg.(type) {
	case tea.WindowSizeMsg:
		m.windowWidth = msg.Width
		m.windowHeight = msg.Height
		m.memoryTable.SetHeight(m.windowHeight / 3) // Dynamic table height based on window size
		return m, nil

	case tickMsg:
		if m.executing {
		    m.vm.execute()
		    m.updateMemoryTable()
		    m.updateDisplayBuffer()
		    return m, tick()
		}
	case tea.KeyMsg:
		switch msg.String() {
		case "q":
			return m, tea.Quit
		case "e":
			m.vm.execute()
			m.updateMemoryTable()
			m.updateDisplayBuffer()
			m.status = "Executed one step"
		case "s":
			m.executing = !m.executing
			if m.executing {
				m.status = "Running"
				return m, tick()
			} else {
				m.status = "Paused"
			}
		case "j", "down":
			m.memoryTable.MoveDown(1)
		case "k", "up":
			m.memoryTable.MoveUp(1)
		}
	}
	return m, nil
}

func tick() tea.Cmd {
    return tea.Tick(time.Second, func(t time.Time) tea.Msg {
	return tickMsg(t)
    })
}

// Update the memory table with the current state
func (m *model) updateMemoryTable() {
	rows := make([]table.Row, 0)
	for addr := 0; addr < 32768; addr += 16 {
		row := make([]string, 17)
		row[0] = fmt.Sprintf("0x%04X", addr) // First column is the base address
		for col := 0; col < 16; col++ {
			row[col+1] = fmt.Sprintf("0x%04X", m.vm.Memory[addr+col])
		}
		rows = append(rows, row)
	}
    m.memoryTable.SetRows(rows)
}

// Update the display buffer based on the memory contents at the "display" register
func (m *model) updateDisplayBuffer() {
	displayAddr := m.vm.Video // Assume "Video" register points to display memory
	for i := 0; i < len(m.displayBuffer); i++ {
		m.displayBuffer[i] = byte(m.vm.Memory[int(displayAddr)+i] & 0xFF) // Take lower byte as ASCII
	}
}

// View function creates a panel-based layout with ASCII display and memory on top,
// and registers and controls below
func (m model) View() string {
	topPanel := lipgloss.JoinHorizontal(
		lipgloss.Top,
		m.displayPanel(), // Left panel: ASCII display
		m.memoryPanel(),  // Right panel: memory
	)

	bottomPanel := lipgloss.JoinHorizontal(
		lipgloss.Top,
		m.registersPanel(),  // Left panel: registers
		m.controlsPanel(),    // Right panel: controls and status
	)

	return topPanel + "\n" + bottomPanel
}

// Panel for displaying memory
func (m model) memoryPanel() string {
	return borderStyle.Width(m.windowWidth / 2).Render("Memory\n" + m.memoryTable.View())
}

// Panel for displaying registers
func (m model) registersPanel() string {
	registers := "Registers:\n"
	for i := 0; i < len(m.vm.Registers); i++ {
		registers += fmt.Sprintf("R%d: 0x%04X  ", i+1, m.vm.Registers[i])
		if (i+1)%4 == 0 {
			registers += "\n"
		}
	}
	registers += fmt.Sprintf("A: 0x%04X  PC: 0x%04X  SP: 0x%04X\n", m.vm.Accumulator, m.vm.PC, m.vm.SP)
	return borderStyle.Width(m.windowWidth / 2).Render(registers)
}

// Panel for displaying the 64x64 ASCII characters
func (m model) displayPanel() string {
	rows := make([]string, 32)
	for i := 0; i < 32; i++ {
		row := ""
		for j := 0; j < 32; j++ {
			row += string(m.displayBuffer[i*32+j]) // Display ASCII characters
		}
		rows[i] = row
	}
	display := lipgloss.NewStyle().Render(lipgloss.JoinVertical(lipgloss.Left, rows...))
	return borderStyle.Width(m.windowWidth / 2).Render("ASCII Display\n" + display)
}

// Panel for displaying controls and status
func (m model) controlsPanel() string {
	flags := fmt.Sprintf("Flags - Z: %v G: %v L: %v E: %v", m.vm.Z, m.vm.G, m.vm.L, m.vm.E)
	controls := "[s] Start/Stop | [e] Step | [q] Quit | Arrow or hjkl to scroll"
	return borderStyle.Width(m.windowWidth / 2).Render(fmt.Sprintf("%s\n\nStatus: %s\n\n%s", flags, m.status, controls))
}

func main() {
	p := tea.NewProgram(NewModel(), tea.WithAltScreen()) // Fullscreen mode
	if err := p.Start(); err != nil {
		fmt.Fprintln(os.Stderr, "Error running program:", err)
		os.Exit(1)
	}
}
