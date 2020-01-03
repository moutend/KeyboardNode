package app

import (
	"fmt"
	"net/http"
	"sync"
	"unsafe"

	"github.com/moutend/KeyboardNode/pkg/dll"
)

type app struct {
	m         *sync.Mutex
	wg        *sync.WaitGroup
	server    *http.Server
	isRunning bool
}

func (a *app) setup() error {
	var code int32

	dll.ProcSetup.Call(uintptr(unsafe.Pointer(&code)))

	if code != 0 {
		return fmt.Errorf("Failed to call dll.ProcSetup.Call()")
	}

	return nil
}

func (a *app) Setup() error {
	a.m.Lock()
	defer a.m.Unlock()

	if a.isRunning {
		fmt.Errorf("Setup is already done")
	}
	if err := a.setup(); err != nil {
		return err
	}

	a.isRunning = true

	return nil
}

func (a *app) teardown() error {
	var code int32

	dll.ProcTeardown.Call(uintptr(unsafe.Pointer(&code)))

	if code != 0 {
		return fmt.Errorf("Failed to call dll.ProcTeardown.Call()")
	}

	return nil
}

func (a *app) Teardown() error {
	a.m.Lock()
	defer a.m.Unlock()

	if !a.isRunning {
		return fmt.Errorf("Teardown is already done")
	}
	if err := a.teardown(); err != nil {
		return err
	}

	a.isRunning = false

	return nil
}

func New() *app {
	return &app{
		m:  &sync.Mutex{},
		wg: &sync.WaitGroup{},
	}
}
