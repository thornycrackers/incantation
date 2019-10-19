package main

// #cgo linux LDFLAGS: -lX11 -lXtst -lXi
// #include "keyboard_writer.h"
// #include "keyboard_reader.h"
import "C"

import (
	"errors"
	"fmt"
	"github.com/BurntSushi/toml"
	"github.com/sirupsen/logrus"
	"reflect"
	"strings"
	"time"
)

type tomlConfig struct {
	Bindings map[string]binding
}

type binding struct {
	Keypress string
	Text     string
}

func main() {
	// Startup Load the config
	config, err := get_config()
	if err != nil {
		logrus.Println(err)
		return
	}
	for {
		expansion := wait_for_autocomplete(config)
		logrus.Println(fmt.Sprintf("executing: %s", expansion))
		time.Sleep(100 * time.Millisecond)
		for _, v := range expansion {
			cs := C.CString(char_to_x11(string(v)))
			if stringInSlice(string(v), needsShift) {
				C.x11_key_shift(cs)
			} else {
				C.x11_key(cs)
			}
		}
	}
}

func get_config() (tomlConfig, error) {
	var config tomlConfig
	if _, err := toml.DecodeFile("incantation.toml", &config); err != nil {
		return config, errors.New("Can't load config file")
	}
	return config, nil
}

func wait_for_autocomplete(config tomlConfig) string {
	max_length := 10
	buffer := make([]string, max_length)
	for {
		key_press := x11_to_char(C.GoString(C.get_key()))
		// Cut down length
		if len(buffer) == max_length {
			buffer = buffer[1:]
		}
		buffer = append(buffer, key_press)
		logrus.Println(strings.Join(buffer, ", "))
		// Check for expansion matches, windowing through buffer
		for _, binding := range config.Bindings {
			// Build kepress string into slice for slice comparison
			keypress_slice := make([]string, 0)
			for _, v := range binding.Keypress {
				keypress_slice = append(keypress_slice, string(v))
			}
			for i := 0; i < len(buffer)-len(keypress_slice)+1; i++ {
				tmp_buf := buffer[i : i+len(keypress_slice)]
				if reflect.DeepEqual(tmp_buf, keypress_slice) {
					// Erase the keypresses
					for range binding.Keypress {
						cs := C.CString("BackSpace")
						C.x11_key(cs)
					}
					return binding.Text
				}
			}
		}
	}
}

func char_to_x11(code string) string {
	for k, v := range x11Map {
		if k == code {
			return v
		}
	}
	return code
}

func x11_to_char(code string) string {
	for k, v := range x11Map {
		if v == code {
			return k
		}
	}
	return code
}

func stringInSlice(a string, list []string) bool {
	for _, b := range list {
		if b == a {
			return true
		}
	}
	return false
}
