# Git Apply

Apply git patches with optional failure handling and summary reporting.

## Usage

```yaml
- uses: openDAQ/openDAQ-CI-sandbox/.github/actions/git-apply@actions/git-apply
  with:
    # Directory to apply patches in
    # Default: .
    project-dir: ''

    # Patch paths (newline-separated, supports wildcards)
    patches: |
      _sandbox/.github/patches/*.patch

    # Continue applying patches after failure instead of stopping
    # Default: false
    continue-on-error: ''

    # Disable writing summary to job summary
    # Default: false
    disable-summary: ''
```

## Outputs

```yaml
# Found patches (newline-separated)
patches: ''

# Applied patches (newline-separated)
applied: ''

# Failed patches (newline-separated)
failed: ''

# Skipped patches (newline-separated, when stopped on failure)
skipped: ''
```
