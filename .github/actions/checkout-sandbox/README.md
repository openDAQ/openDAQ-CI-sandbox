# Checkout Sandbox

Checkout project and sandbox repositories, copy CI infrastructure.

## Usage

```yaml
- uses: openDAQ/openDAQ-CI-sandbox/.github/actions/checkout-sandbox@actions/checkout-sandbox
  with:
    # Project repository
    # Default: openDAQ/openDAQ
    project-repo: ''

    # Branch, tag, or SHA to checkout
    project-ref: ''

    # Directory to checkout sandbox into
    # Default: _sandbox
    sandbox-dir: ''

    # Copy actions from sandbox to project
    # Default: true
    copy-actions: ''

    # Copy workflows from sandbox to project
    # Default: false
    copy-workflows: ''
```

## Outputs

```yaml
# Path to sandbox directory
sandbox-dir: ''
```
