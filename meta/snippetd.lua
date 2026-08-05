---@meta

---@class SnippetSpec
---@field trigger string          Text that triggers the expansion
---@field expand  string|function():string  Replacement text, or a function returning it

--- Register a snippet.
---@param spec SnippetSpec
function snippet(spec) end

--- Register several snippets at once.
---@param specs SnippetSpec[]
function snippets(specs) end
