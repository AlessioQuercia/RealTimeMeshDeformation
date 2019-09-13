.PHONY: clean All

All:
	@echo "----------Building project:[ Feedback - Debug ]----------"
	@cd "work\Feedback" && "$(MAKE)" -f  "Feedback.mk" && "$(MAKE)" -f  "Feedback.mk" PostBuild
clean:
	@echo "----------Cleaning project:[ Feedback - Debug ]----------"
	@cd "work\Feedback" && "$(MAKE)" -f  "Feedback.mk" clean
