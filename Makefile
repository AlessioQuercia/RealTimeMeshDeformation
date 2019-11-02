.PHONY: clean All

All:
	@echo "----------Building project:[ geometry - Debug ]----------"
	@cd "work\geometry" && "$(MAKE)" -f  "geometry.mk" && "$(MAKE)" -f  "geometry.mk" PostBuild
clean:
	@echo "----------Cleaning project:[ geometry - Debug ]----------"
	@cd "work\geometry" && "$(MAKE)" -f  "geometry.mk" clean
