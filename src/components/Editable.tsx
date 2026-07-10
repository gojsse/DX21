import { useEffect, useRef, useState } from 'react'
import type { CSSProperties } from 'react'

interface Props {
  value: string
  onCommit?: (v: string) => void
  style?: CSSProperties
  title?: string
}

/**
 * A value readout that becomes a text field on double-click and commits on
 * Enter / blur (Escape cancels). Every draggable control is also typeable
 * per the handoff — this is the "typeable" half.
 */
export function Editable({ value, onCommit, style, title }: Props) {
  const [editing, setEditing] = useState(false)
  const [draft, setDraft] = useState(value)
  const inputRef = useRef<HTMLInputElement>(null)

  useEffect(() => { setDraft(value) }, [value])
  useEffect(() => { if (editing) { inputRef.current?.focus(); inputRef.current?.select() } }, [editing])

  const commit = () => {
    setEditing(false)
    if (onCommit && draft !== value) onCommit(draft)
  }

  if (editing && onCommit) {
    return (
      <span style={style}>
        <input
          ref={inputRef}
          className="op4-edit"
          value={draft}
          onChange={(e) => setDraft(e.target.value)}
          onBlur={commit}
          onKeyDown={(e) => {
            if (e.key === 'Enter') commit()
            else if (e.key === 'Escape') { setDraft(value); setEditing(false) }
          }}
        />
      </span>
    )
  }

  return (
    <span
      style={{ ...style, cursor: onCommit ? 'text' : undefined }}
      title={title ?? (onCommit ? 'Double-click to edit' : undefined)}
      onDoubleClick={() => onCommit && setEditing(true)}
    >
      {value}
    </span>
  )
}
