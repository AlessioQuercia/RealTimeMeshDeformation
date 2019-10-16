.PHONY: clean All

All:
	@echo "----------Building project:[ FeeFeed - Debug ]----------"
	@cd "work\FeeFeed" && "$(MAKE)" -f  "FeeFeed.mk" && "$(MAKE)" -f  "FeeFeed.mk" PostBuild
clean:
	@echo "----------Cleaning project:[ FeeFeed - Debug ]----------"
	@cd "work\FeeFeed" && "$(MAKE)" -f  "FeeFeed.mk" clean
