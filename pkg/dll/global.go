package dll

import (
	"syscall"
)

var (
	dll = syscall.NewLazyDLL("KeyboardNode.dll")

	ProcSetup    = dll.NewProc("Setup")
	ProcTeardown = dll.NewProc("Teardown")
)
